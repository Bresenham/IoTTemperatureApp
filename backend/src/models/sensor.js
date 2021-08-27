const mongoose = require('mongoose');

const SensorType = Object.freeze({
    Temperature: "Temperature"
});

const SensorSchema = new mongoose.Schema({
    id: { type: Number, required: true },
    type: { type: Object.values(SensorType), required: true },
});

Object.assign(SensorSchema.statics, { SensorType });

const Sensor = mongoose.model('Sensor', SensorSchema);

module.exports = { SensorSchema, Sensor };
