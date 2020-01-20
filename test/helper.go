package testsuite

import (
	"fmt"
	"io"
	"os/exec"
	"strings"
	"testing"

	"github.com/sergi/go-diff/diffmatchpatch"
)

// Run executes the json-fmt utility with the given command line arguments.
// Input is passed to the utility via an io.Reader which is bound to the
// process' standard input. After the utility has finished, the contents of
// its standard output and standard error devices are returned as strings along
// with an error if one occurred.
func Run(input io.Reader, args ...string) (string, string, error) {
	cmd := exec.Command("json-fmt", args...)
	cmd.Dir = "../build"
	cmd.Stdin = input
	stdout := new(strings.Builder)
	cmd.Stdout = stdout
	stderr := new(strings.Builder)
	cmd.Stderr = stderr
	err := cmd.Run()
	return stdout.String(), stderr.String(), err
}

// ExpectEq checks to see if expected and actual are equal and logs an error
// with the testing framework if they are not.
func ExpectEq(t *testing.T, expected, actual string) {
	format := "\n==== Expected ====\n%v\n==================\n\n" +
		"====  Actual  ====\n%v\n==================\n\n====   Diff   ====\n%v\n"

	// Don't care about leading linebreaks so we'll trim them out.
	e := strings.Trim(expected, "\n")
	a := strings.Trim(actual, "\n")
	
	if e != a {
		dmp := diffmatchpatch.New()
		diffs := dmp.DiffMain(a, e, true)
		fmt.Println(diffs)
		t.Errorf(format, e, a, dmp.DiffPrettyText(diffs))
	}
}

// RunTest executes a single test on the json-fmt utility with some given input
// and expected output.
func RunTest(t *testing.T, input, expected string) {
	reader := strings.NewReader(input)
	stdout, _, err := Run(reader)
	if err != nil {
		t.Fatal(err)
	}

	ExpectEq(t, expected, stdout)
}
