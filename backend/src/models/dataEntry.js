const mongoose = require('mongoose');
const Sensor = require('../models/sensor');

const DataEntrySchema = new mongoose.Schema({
    datetime: { type: mongoose.Schema.Types.String, required: true },
    sensor: { type: mongoose.Schema.Types.Number, ref: 'Sensor', required: true },
    value: { type: mongoose.Schema.Types.Number, required: true }
});
  
const DataEntry = mongoose.model('DataEntry', DataEntrySchema);

module.exports = DataEntry;
