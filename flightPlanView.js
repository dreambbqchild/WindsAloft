class FlightPlanView {
	flightPlan;
	
	constructor(flightPlan) {
		this.flightPlan = flightPlan;
	}
	
	getForecastOptions() {		
		return this.flightPlan.model.forecastMetadata.map((metadata, i)=> {
			var option = document.createElement('option');
			option.value = i;
			option.text = (metadata.time.getMonth() + 1) + "/" + metadata.time.getDay() + " " + metadata.time.getHours() + ":00";
			return option;
		});
	}
	
	getTableRows(forecastHour, altitude) {
		function toDomObject(str) {
			var tr = document.createElement('tr');
			tr.innerHTML = str;
			return tr;
		}
	
		function rows(subResult) {	
			return [
				toDomObject(`<td rowspan="2">${altitude}</td>
				<td>${subResult.windDirection}</td>
				<td>${subResult.windSpeed}</td>
				<td rowspan="2">${subResult.trueAirspeed}</td>
				<td rowspan="2">${subResult.groundSpeed}</td>
				<td>${subResult.trueCourse}</td>
				<td>${subResult.trueHeading}</td>
				<td>${subResult.magneticHeading}</td>
				<td>${subResult.distanceTraveled}</td>`),
				toDomObject(`<td colspan="2">${subResult.temp}</td>
				<td>${subResult.windCorrectionAngle}</td>
				<td>${subResult.magneticVariation}</td>
				<td></td>
				<td>${subResult.distanceToGo}</td>`)
			];
		}
	
		var result = [];
		var hourComplete = 0;
		for(var i = 0; i < flightPlan.numberOfCheckpoints() - 1; i++) {
			if(hourComplete >= 1) {
				hourComplete = 0;
				forecastHour++;
			}
			
			var subResult = flightPlan.getModelValuesFor(forecastHour, i, altitude);
			hourComplete += 10 / subResult.groundSpeed;
			result = result.concat(rows(subResult));
		}
		
		return result;
	}
}
