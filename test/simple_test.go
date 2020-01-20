package testsuite

import (
	"testing"
)

func TestSimpleObject(t *testing.T) {
	input := `{"a":"b"}`
	expected := `{
  "a":"b"
}`
	RunTest(t, input, expected)
}

func TestSimpleArray(t *testing.T) {
	input := `["a",2,null]`
	expected := `[
  "a",
  2,
  null
]`
	RunTest(t, input, expected)
}
