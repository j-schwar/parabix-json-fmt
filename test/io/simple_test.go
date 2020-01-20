package io

import (
	"testing"

	"github.com/j-schwar/parabix-json-fmt/testsuite"
)

func TestSimpleObject(t *testing.T) {
	input := `{"a":"b"}`
	expected := `
{
  "a":"b"
}`
	testsuite.RunTest(t, input, expected)
}

func TestSimpleArray(t *testing.T) {
	input := `["a",2,null]`
	expected := `
[
  "a",
  2,
  null
]`
	testsuite.RunTest(t, input, expected)
}
