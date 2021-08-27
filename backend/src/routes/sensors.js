const router = require('express').Router();
const { SensorSchema, Sensor } = require('../models/sensor');

router.route('/').get((req, res) => {
    Sensor.find().then( sensors => res.json(sensors) ).catch(err => res.status(400).json('Error: ' + err));
});

module.exports = router;
