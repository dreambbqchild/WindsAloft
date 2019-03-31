class FlightPlan {
	model = {};
	
	constructor(from, to, trueAirspeed) {
		var request = new XMLHttpRequest();
		var self = this;
		request.addEventListener("load", function() {
			self.model = JSON.parse(request.responseText);
		});
		request.open("GET", `cgi-bin/flightPlan.sh?from=${from}&to=${to}&trueAirspeed.cruise=${trueAirspeed}`);
		request.send();
	}
	
	numberOfCheckpoints() {
		return model.forecasts[0].checkpts.length;
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
		result.magneticHeading = (result.trueHeading + metadata.magVar) % 360;
		result.latitude = metadata.lat;
		result.longitude = metadata['long'];
		return result;
	}
}