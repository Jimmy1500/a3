'use strict'
const { EventEmitter } = require('events');
const Redis = require('ioredis');
const { redis } = require("./redis")
const { parser } = require("./parser")

var visited_ = 0;
var checked_ = 0;
const emitter_ = new EventEmitter();
const redis_ = new Redis();

const topic_ = {
    home: 'home',
    health: 'health',
    stats: 'stats_okay',
    no_stats: 'no_stats'
};

emitter_.on(
    topic_.home,
    res => {
        ++visited_;
        redis_.xadd(topic_.home, "*", "visited", visited_).then(
            id => { console.log(`event ${id} added to redis stream ${topic_.home}`); }
        );
        res.status(200).send(`<h1>Hello World, visited ${visited_}<h1>`);
    }
);

emitter_.on(
    topic_.health,
    res => {
        ++checked_;
        redis_.xadd(topic_.health, "*", "checked", checked_).then(
            id => { console.log(`event ${id} added to redis stream ${topic_.health}`); }
        );
        res.status(200).send(`<h1>App is online, health checked ${checked_}<h1>`);
    }
);

emitter_.on(
    topic_.stats,
    (res, home, health) => {
        res.status(200).send(`<div><h1>home: ${home.val} ${home.key}</h1><h1>health: ${health.val} ${health.key}</h1></div>`)
    }
)

emitter_.on(
    topic_.no_stats,
    (res) => {
        res.status(404).send(`<div><h1>home: 0 visited</h1><h1>health: 0 checked</h1></div>`)
    }
)

module.exports = {
    a3_: a3,
    emitter_,
    redis_,
    topic_
}