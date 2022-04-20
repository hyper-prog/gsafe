/* Command line Json to Yaml converter, configured for gSAFE
 * (C) 2022 Péter Deák (hyper80@gmail.com)
 * License: Apache-2.0
 */
package main

import (
	"fmt"
	"github.com/hyper-prog/smartjson"
	"io/ioutil"
	"os"
)

func main() {

	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "Missing parameter, usage: jsontoyaml <JSONFILE>")
		return
	}

	jsonData, flerr := ioutil.ReadFile(os.Args[1])
	if flerr != nil {
		fmt.Fprintln(os.Stderr, "Error, cannot read file: ", flerr.Error())
		return
	}

	sj, parsererror := smartjson.ParseJSON(jsonData)
	sj.Config.OutputMapKeyOrder = []string{
		"name", "sqlname", "type", "description", "title", "unknownallowed", "conntable", "connkey",
		"connshow", "connfilter", "connsort", "default", "value", "selectables", "unlistedSelectables",
		"nosql", "mode", "displayflags", "attributes", "tags", "fields", "alternate_actions"}

	if parsererror != nil {
		fmt.Fprintln(os.Stderr, "Error, not valid JSON: ", parsererror.Error())
		return
	}

	fmt.Println(sj.Yaml())
}
