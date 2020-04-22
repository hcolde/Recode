package main

import (
	"bufio"
	"flag"
	"github.com/axgle/mahonia"
	"golang.org/x/text/encoding/unicode"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"
)

var decoderLE = unicode.UTF16(unicode.LittleEndian, unicode.UseBOM).NewDecoder()
var decoderBE = unicode.UTF16(unicode.BigEndian, unicode.UseBOM).NewDecoder()

type Result struct {
	fpath string
	code int
	err error
}

func main() {
	var (
		source string
		output string
	)
	flag.StringVar(&source, "source", "", "set source path")
	flag.StringVar(&output, "output", "", "set output path")
	flag.Parse()

	Run(source, output)

}

func Run(source, output string) {
	_ = os.Remove("./recoding.log")
	log.SetPrefix("[recoding]")
	log.SetFlags(log.Ldate|log.Ltime|log.Lshortfile)
	logFile, err := os.OpenFile("./recoding.log", os.O_RDWR|os.O_CREATE, 0666)
	if err != nil {
		log.Println(err.Error())
		return
	}
	defer logFile.Close()
	log.SetOutput(logFile)

	if source == "" || output == "" {
		log.Println("Please set source and output param")
		return
	}

	dir, _ := filepath.Split(source)

	if dir == output || dir == strings.Join([]string{output, "/"}, "") || dir == strings.Join([]string{output, "\\"}, "") {
		log.Println("output path could not in source path")
		return
	}

	if _, err := os.Stat(output); err == nil {
		if err := os.RemoveAll(output); err != nil {
			log.Println(err.Error())
			return
		}
	}

	if err := os.MkdirAll(output, os.ModePerm); err != nil {
		log.Println(err.Error())
		return
	}

	var files []string
	if isFile(source) {
		files = []string{source}
	} else {
		files = getFilePath(source, source, output, files)
	}
	length := len(files)

	ch := make(chan Result)
	for _, f := range files {
		o := filepath.Join(output, strings.Replace(f, source, "", -1))
		go modifyCoding(f, o, ch)
	}

	for i := 0; i < length; i++ {
		r := <-ch
		msg := "不用转换"
		if r.code == 2 {
			msg = "转换成功"
		} else if r.code == 3 {
			msg = strings.Join([]string{"转换失败.err:", r.err.Error()}, "")
		}
		log.Println(r.fpath, msg)
	}
}

func isFile(str string) bool {
	file, err := os.Stat(str)
	if err != nil {
		return false
	}

	return !file.IsDir()
}

func getFilePath(fileDir, source, output string, ret []string) []string {
	dir, err := ioutil.ReadDir(fileDir)
	if err != nil {
		return nil
	}

	for _, fi := range dir {
		if fi.Name()[0] == 46 {
			continue
		} else if fi.IsDir() {
			o := filepath.Join(output, strings.Replace(filepath.Join(fileDir, fi.Name()), source, "", -1))
			err = os.MkdirAll(o, os.ModePerm)
			if err != nil {
				log.Println(err.Error())
				continue
			}
			ret = getFilePath(filepath.Join(fileDir, fi.Name()), source, output, ret)
		} else {
			ret = append(ret, filepath.Join(fileDir, fi.Name()))
		}
	}
	return ret
}

func modifyCoding(source, output string, ch chan Result) {
	r := Result{output, 1, nil}
	f, err := os.Open(source)

	if err != nil {
		r.code = 3
		r.err = err
		ch <- r
		return
	}

	defer f.Close()

	buffer := make([]byte, 3)
	if _, err := f.Read(buffer); err != nil {
		if err != io.EOF {
			r.code = 3
			r.err = err
			ch <- r
			return
		}
	}


	ns := 0

	if buffer[0] == 255 {
		ns = 1
	}else if buffer[0] == 254 {
		ns = 2
	}else if buffer[0] == 239 && buffer[1] == 187 && buffer[2] == 191 {
		ns = 3
	}

	f2, err := os.OpenFile(output, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0666)
	if err != nil {
		r.code = 3
		r.err = err
		ch <- r
		return
	}
	defer f2.Close()

	if ns != 3 {
		_, _ = f.Seek(0, 0)
	}

	code := 1
	buf := bufio.NewReader(f)
	bk := false
	for {
		b, err := buf.ReadBytes('\n')
		if err != nil {
			if err == io.EOF {
				bk = true
			} else {
				break
			}
		}

		buffer, code2 := getUtf8(ns, b)
		if code2 > code {
			code = code2
		}
		_, _ = f2.Write(buffer)
		if bk {
			break
		}
	}

	if ns == 3 {
		code = 2
	}
	r.code = code
	ch <- r
}

func getUtf8(num int, bs []byte) ([]byte, int) {
	switch num {
	case 1:
		bs, _ = decoderLE.Bytes(bs)
		return bs, 2
	case 2:
		bs, _ = decoderBE.Bytes(bs)
		return bs, 2
	case 3:
		return bs, 1
	default:
		if !isGBK(bs) || isUTF8(bs) {
			return bs, 1
		}
	}

	decoder := mahonia.NewDecoder("gbk")
	_, bs, _ = decoder.Translate(bs, true)
	return bs, 2
}

func isGBK(data []byte) bool {
	length := len(data)
	i := 0
	r := false
	for i < length {
		if data[i] < 0x80 { // ascii
			i++
		} else if i + 1 < length { // 使用双字节
			if (data[i] < 0x81 || data[i] > 0xFE) && (data[i + 1] < 0x40 || data[i + 1] > 0xFE) {
				return false
			}
			i += 2
			r = true
		} else if data[i] < 0x40 || data[i] > 0xFE { // 使用双字节 并且高位为0
			return false
		} else {
			i++
			r = true
		}
	}
	return r
}

func isUTF8(data []byte) bool {
	count := 0 // 后面有多少位变长
	for _, i := range data {
		if count <= 0 { // 计算后面有多少位变长
			if i < 0x80 { // ascii
				count = 0
				continue
			}
			temp := i
			for temp >> 7 > 0 { // 判断比特位最高位是否为1
				temp = temp << 1 & 0xf0
				count++
			}
			count-- // 要算上自身这一位
			continue
		} else if i >> 6 != 2 {
			return false
		}
		count--
	}
	return true
}
