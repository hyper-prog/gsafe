/* Command line Yaml to Json converter, configured for gSAFE
 * (C) 2022 Péter Deák (hyper80@gmail.com)
 * License: Apache-2.0
 */
package main

import (
	"fmt"
	"github.com/hyper-prog/smartyaml"
	"io/ioutil"
	"os"
)

func main() {

	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "Missing parameter, usage: yamltojson <YAMLFILE>")
		return
	}

	yamlData, flerr := ioutil.ReadFile(os.Args[1])
	if flerr != nil {
		fmt.Fprintln(os.Stderr, "Error, cannot read file: ", flerr.Error())
		return
	}

	sy, parsererror := smartyaml.ParseYAML(yamlData)
	sy.Config.OutputMapKeyOrder = []string{
		"name", "sqlname", "type", "description", "title", "unknownallowed", "conntable", "connkey",
		"connshow", "connfilter", "connsort", "default", "value", "selectables", "unlistedSelectables",
		"nosql", "mode", "displayflags", "attributes", "tags", "fields", "alternate_actions"}

	if parsererror != nil {
		fmt.Fprintln(os.Stderr, "Error, not valid YAML: ", parsererror.Error())
		return
	}

	fmt.Println(sy.JsonIndented())
}
