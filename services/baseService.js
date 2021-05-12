'use strict'
const { a3_, emitter_, redis_, topic_ } = require("../lib/async");

const consume = async (topic) => {
    const streams = await redis_.xread("block", "100", "STREAMS", topic, "0");
    if ( streams ) {
        const [ key, msgs ] = streams[0]; // there is only 1 stream (per topic) at most
        if ( msgs.length ) {
            let id, data;
            msgs.forEach(
                msg => {
                    const [ id, data ] = msg;
                    console.log(key, data);
                    redis_.xdel(key, id);
                }
            );
            return data;
        }
    }
    return null;
}

const home = async (req, res) => {
    emitter_.emit(topic_.home, res);
}

const health = async (req,res) => {
    emitter_.emit(topic_.health, res);
}

var home_ = { id: '0', key: 'visited', val: 0 }, health_ = {id: '0', key: 'checked', val: 0 };
const stats = async (req, res) => {
    if ( !home_.val || !health_.val ) {
        const accepted = a3_.consumeAsync( topic_.home, topic_.health, 
            ( message, okay, visited, checked ) => {
                if ( okay ) {
                    if ( visited.val ) { home_ = visited; }
                    if ( checked.val ) { health_ = checked; }
                    emitter_.emit(topic_.stats, res, home_, health_);
                    console.log("success: ", message);
                } else {
                    emitter_.emit(topic_.no_stats, res);
                    console.error("failure: ", message);
                }
            }
        );
        if ( accepted.okay ) { console.log( accepted.message ); }
        else { console.error( accepted.message ); }

    } else {
        emitter_.emit(topic_.stats, res, home_, health_);
    }

    // const home = await consume(topic_.home);
    // const health = await consume(topic_.health);
}

module.exports = {
    home,
    health,
    stats
}