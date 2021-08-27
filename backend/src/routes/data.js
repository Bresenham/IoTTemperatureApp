const router = require('express').Router();
const DataEntry = require('../models/dataEntry');

router.route('/').get((req, res) => {
    DataEntry.find().then( dataEntries => res.json(dataEntries) ).catch(err => res.status(400).json('Error: ' + err));
});

module.exports = router;
