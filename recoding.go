package main

import "C"
import (
	"bufio"
	"encoding/json"
	"github.com/axgle/mahonia"
	zmq "github.com/pebbe/zmq4"
	"golang.org/x/text/encoding"
	"golang.org/x/text/encoding/charmap"
	"golang.org/x/text/encoding/unicode"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

type Result struct {
	fpath string
	code int
	err string
}

type Data struct {
	Rate string `json:"rate"`
	Msg string `json:"msg"`
}

func main() {
}

func setResult(fpath string, code int, err string) (result Result) {
	result.code = code
	result.fpath = fpath
	result.err = err
	return
}

func server(data chan Data, stop chan bool) {
	responder, _ := zmq.NewSocket(zmq.REP)
	defer responder.Close()
	_ = responder.Bind("tcp://127.0.0.1:12138")
	bk := 0

	for {
		identity, err := responder.Recv(0)
		if bk == 1 {
			break
		} else if err != nil {
			continue
		}

		if identity != "recoding" {
			continue
		}

		d := <-data
		js, err := json.Marshal(d)
		if err != nil {
			_, _ = responder.Send("", 0)
			continue
		}

		_, _ = responder.SendBytes(js, 0)
		rate, _ := strconv.Atoi(d.Rate)
		if rate >= 100 || rate == -1 {
			bk = 1
		}
	}
	stop <- true
}

//export Recode
func Recode(source, desktop string) {
	stop := make(chan bool, 1)
	data := make(chan Data, 10)
	go server(data, stop)
	defer func() {
		<- stop
	}()

	output := filepath.Join(desktop, "output")
	if source == "" || output == "" {
		data <- Data{"-1", "Please set source and output param"}
		return
	}

	dir, _ := filepath.Split(source)

	if dir == output || dir == strings.Join([]string{output, "/"}, "") || dir == strings.Join([]string{output, "\\"}, "") {
		data <- Data{"-1", "output path could not in source path"}
		return
	}

	if _, err := os.Stat(output); err == nil {
		if err := os.RemoveAll(output); err != nil {
			data <- Data{"-1", err.Error()}
			return
		}
	}

	if err := os.MkdirAll(output, os.ModePerm); err != nil {
		data <- Data{"-1", err.Error()}
		return
	}

	var files []string
	if isFile(source) {
		files = []string{source}
	} else {
		files = getFilePath(source, source, output, files, data)
	}
	if files == nil {
		return
	}
	length := len(files)

	ch := make(chan Result)
	decoderLE := unicode.UTF16(unicode.LittleEndian, unicode.UseBOM).NewDecoder()
	decoderBE := unicode.UTF16(unicode.BigEndian, unicode.UseBOM).NewDecoder()
	for _, f := range files {
		o := strings.Replace(f, source, output, -1)
		if source == f {
			_, t :=filepath.Split(f)
			o = filepath.Join(output, t)
		}
		go modifyCoding(f, o, ch, decoderLE, decoderBE)
	}

	for i := 0; i < length; i++ {
		r := <-ch
		msg := "no need"
		if r.code == 2 {
			msg = "succeeded"
		} else if r.code == 3 {
			msg = strings.Join([]string{"failed.err:", r.err}, "")
		}
		msg = strings.Join([]string{"[", r.fpath, "]:", msg}, "")
		data <- Data{strconv.Itoa((i + 1) / length * 100), msg}
	}
}

func isFile(str string) bool {
	file, err := os.Stat(str)
	if err != nil {
		return false
	}

	return !file.IsDir()
}

func getFilePath(fileDir, source, output string, ret []string, data chan Data) []string {
	dir, err := ioutil.ReadDir(fileDir)
	if err != nil {
		data <- Data{"-1", err.Error()}
		return nil
	}

	for _, fi := range dir {
		if fi.Name()[0] == 46 {
			continue
		} else if fi.IsDir() {
			o := filepath.Join(output, strings.Replace(filepath.Join(fileDir, fi.Name()), source, "", -1))
			err = os.MkdirAll(o, os.ModePerm)
			if err != nil {
				data <- Data{"-1", err.Error()}
				return nil
			}
			ret = getFilePath(filepath.Join(fileDir, fi.Name()), source, output, ret, data)
		} else {
			ret = append(ret, filepath.Join(fileDir, fi.Name()))
		}
	}
	return ret
}

/*
* code:
*   1: no need
*   2: success
*   3: failed
*/
func modifyCoding(source, output string, ch chan Result, decoderLE, decoderBE *encoding.Decoder) {
	f, err := os.Open(source)

	if err != nil {
		ch <- setResult(output, 3, err.Error())
		return
	}

	defer f.Close()

	buffer := make([]byte, 3)
	if _, err := f.Read(buffer); err != nil {
		if err != io.EOF {
			ch <- setResult(output, 3, err.Error())
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
		ch <- setResult(output, 3, err.Error())
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

		buffer, code2 := getUtf8(ns, b, decoderLE, decoderBE)
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
	ch <- setResult(output, code, "")
}

func getUtf8(num int, bs []byte, decoderLE, decoderBE *encoding.Decoder) ([]byte, int) {
	isgbk := false
	isiso8859_1 := false
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
		if isGBK(bs) {
			isgbk = true
		} else if isUTF8(bs) {
			return bs, 1
		} else {
			isiso8859_1 = true
		}
	}

	if isgbk {
		decoder := mahonia.NewDecoder("gbk")
		_, bs, _ = decoder.Translate(bs, true)
		return bs, 2
	} else if isiso8859_1 {
        latin1, err := charmap.ISO8859_1.NewDecoder().Bytes(bs)
        if err != nil {
            return bs, 1
        }
        return latin1, 2
    }
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
