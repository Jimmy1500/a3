'use strict'
const { a3 } = require('.');

const { EventEmitter } = require('events')
const signal = new EventEmitter();
const event = { a3Bulk: 'a3Bulk', a3: 'a3', xml2js: 'xml2js' }

var a3Objs = []
async function a3ParseXML( xmls, display = false ) {
    const rejected = a3.parse( xmls,
        (err, obj) => {
            if ( !err ) {
                if ( Array.isArray(obj) ) {
                    signal.emit(event.a3Bulk, obj.length)
                } else {
                    a3Objs.push(obj);
                    if ( a3Objs.length == bulk_size ) { signal.emit(event.a3, bulk_size) }
                }
                if ( display ) { console.log(JSON.stringify(obj)) }
            } else {
                console.error(err)
            }
        }
    );
    if ( rejected ) { console.error(rejected) }
}

/*
# xml2js.Parser().parse() vs. a3.parse()
* parse speed:
    * a3 is 6-7 times faster than xml2js
    * the gap widens as work load further scales up
* parse output:
    * a3 produced json is more human readable
    * a3 produced json is less verbose (further reduces network cost, e.g. axios calls, database operations)
    * a3 produced json is more intuitive/faster to analyze (further reduces computation cost, e.g. obj look ups)
*/
const fs = require('fs');
const xml2js = require('xml2js');
const parser = new xml2js.Parser();
var xml2jsObjs = []

async function xml2jsParseXML( xml_file, display = false ) {
    fs.readFile( xml_file,
        (err, xml_string) => {
            parser.parseString( xml_string,
                (err, obj) => {
                    if ( !err ) {
                        xml2jsObjs.push(obj);
                        if ( xml2jsObjs.length == bulk_size ) { signal.emit(event.xml2js, bulk_size) }
                        if ( display ) { console.log(JSON.stringify(obj)) }
                    } else {
                        console.error(err)
                    }
                }
            );
        }
    );
}

/* benchmark */
const bulk_size = 1000;
const file = 'nfe.xml';
async function benchmark() {
    signal.on(event.a3Bulk, (bulk) => { console.timeEnd(event.a3Bulk); console.debug(`${event.a3Bulk}: ${bulk} files processed`);  })
    signal.on(event.a3,     (bulk) => { console.timeEnd(event.a3);     console.debug(`${event.a3}: ${bulk} files processed`);      })
    signal.on(event.xml2js, (bulk) => { console.timeEnd(event.xml2js); console.debug(`${event.xml2js}: ${bulk} files processed`);  })

    console.time(event.a3);
    for (var i = 0; i < bulk_size; ++i) { a3ParseXML(file); }

    console.time(event.xml2js);
    for (var i = 0; i < bulk_size; ++i) { xml2jsParseXML(file); }

    let xmls = [];
    for (var i = 0; i < bulk_size; ++i) { xmls.push(file); }
    console.time(event.a3Bulk);
    a3ParseXML(xmls);
}

/* run benchmark */
benchmark();