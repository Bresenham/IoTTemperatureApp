const router = require('express').Router();

const dateFormat = require('dateformat');

const Sensor = require('../models/sensor');
const DataEntry = require('../models/dataEntry');


router.route('/').get((req, res) => {
    Sensor.find().then( sensors => res.json(sensors) ).catch(err => res.status(400).json('Error: ' + err));
});

router.route('/add').post((req, res) => {

    const params = ["id", "value", "auth_key"];

    missing_params = [];
    params.forEach(param => {
        if( !(param in req.body) ) {
            missing_params.push(param);
        }
    });

    if( missing_params.length > 0 ) {
        res.status(400).json("Missing parameters: " + missing_params);
        return;
    }

    const sensor_id = req.body.id;

    Sensor.findById(sensor_id, (err, sensor) => {

        if (err) {

            res.status(400).json("Error: " + err);

        } else if (sensor) {

            const newEntry = new DataEntry(
                {
                    "datetime" : dateFormat(new Date(), "yyyy-mm-dd HH:MM:ss"),
                    "sensor" : sensor._id,
                    "value" : req.body.value
                }
            );

            newEntry.save(function(err, doc) {
                if(err) {
                    res.status(400).json("Error: " + err);
                } else {
                    res.status(201).json(doc);
                }
            })

        } else {
            res.status(400).json("Sensor with id '" + sensor_id + "' doesn't exist.");
        }

    });

});

module.exports = router;
