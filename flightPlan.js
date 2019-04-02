class FlightPlan {
	model = {};
	from = ''; 
	to = ''; 
	trueAirspeed = 0;

	constructor(from, to, indicatedAirspeed) {
		this.from = from;
		this.to = to;
		this.indicatedAirspeed = indicatedAirspeed;
		var self = this;

		this.httpRequest = new Promise(function(resolve) {
			var request = new XMLHttpRequest();
			request.addEventListener("load", function() {
				self.model = JSON.parse(request.responseText, (key, value) => {
					if(key === 'time') {
						var m = value.match(/(\d\d\d\d)(\d\d)(\d\d)(\d\d)/);
						return new Date(Date.UTC(m[1], m[2], m[3], m[4]));
					}
					return value;
				});
				resolve(self);
			});
			request.open("GET", `cgi-bin/flightPlan.sh?from=${from}&to=${to}&indicatedAirspeed=${indicatedAirspeed}`);
			request.send();
		});
	}

	numberOfCheckpoints() {
		return this.model.forecasts[0].checkpts.length;
	}

	interoplate(altitude, altitudeLow, low, altitudeHigh, high) {
		function calc(property) {
			return Math.round((high[property] - low[property]) * percentage + low[property]);
		}

		var range = (altitudeHigh - altitudeLow);
		var percentage = (altitude - altitudeLow) / range;
		return {
			windCorrectionAngle: calc('WCA'),
			groundSpeed: calc('groundSpd'),
			temp: calc('temp'),
			windDirection: calc('windDir'),
			windSpeed: calc('windSpd'),
			trueAirspeed: calc('trueAirspeed')
		};
	}

	getModelValuesFor(forecastHour, checkpointIndex, altitude) {
		var metadata = this.model.checkpointMetadata[checkpointIndex];
		var checkpoint = this.model.forecasts[forecastHour].checkpts[checkpointIndex];
		var altitudeLow = null, altitudeHigh = null;
		var keys = Object.keys(checkpoint.altitudes);

		for(var i = 0; i < keys.length; i++) {
			if(!altitudeLow && keys[i] > altitude)
				altitudeLow = keys[Math.max(0, i - 1)];

			if(altitudeLow && keys[i] > altitude) {
				altitudeHigh = keys[Math.max(0, i)];
				break;
			}
		}

		if(!altitudeLow)
			throw `${altitude} too high`;

		if(altitudeHigh == null)
			altitudeHigh = keys[keys.length - 1];


		var result = this.interoplate(altitude, altitudeLow, checkpoint.altitudes[altitudeLow], altitudeHigh, checkpoint.altitudes[altitudeHigh]);
		result.trueCourse = metadata.trueCourse;
		result.trueHeading = (result.windCorrectionAngle + metadata.trueCourse) % 360;
		result.magneticVariation = metadata.magVar;
		result.magneticHeading = (result.trueHeading + metadata.magVar) % 360;
		result.latitude = metadata.lat;
		result.longitude = metadata['long'];
		result.distanceToGo = Math.round(this.model.nmDistance - (checkpointIndex * 10));
		result.distanceTraveled = Math.round(this.model.nmDistance - result.distanceToGo);
		return result;
	}
}
