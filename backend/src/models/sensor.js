const mongoose = require('mongoose');

const SensorSchema = new mongoose.Schema({
    _id: { type: mongoose.Schema.Types.Number, required: true },
    type: { type: mongoose.Schema.Types.String, required: true },
});

const Sensor = mongoose.model('Sensor', SensorSchema);

module.exports = Sensor;
