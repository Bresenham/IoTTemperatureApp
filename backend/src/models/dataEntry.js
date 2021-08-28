const mongoose = require('mongoose');
const SensorSchema = require('./sensor');

const DataEntrySchema = new mongoose.Schema({
    datetime: { type: String, required: true },
    sensor: { type: SensorSchema, required: true },
    value: { type: Number, required: true }
});
  
const DataEntry = mongoose.model('DataEntry', DataEntrySchema);

module.exports = DataEntry;
