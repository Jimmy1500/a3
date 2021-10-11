'use strict'
const express = require('express');
const app = express();
const { home, health, stats } = require('../services/base');

app.get('/', home);
app.get('/health', health);
app.get('/stats', stats);

// exports.app = app
module.exports = {
    app
}