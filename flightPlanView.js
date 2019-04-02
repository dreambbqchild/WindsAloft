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
	
	getTableRows(forecastHour, checkpointIndex, altitude) {
		function toDomObject(str) {
			var tr = document.createElement('tr');
			tr.innerHTML = str;
			return tr;
		}
		
		var result = flightPlan.getModelValuesFor(forecastHour, checkpointIndex, altitude);
		return [toDomObject(
   `<td rowspan="2">${altitude}</td>
	<td>${result.windDirection}</td>
	<td>${result.windSpeed}</td>
	<td rowspan="2">${result.trueAirspeed}</td>
	<td rowspan="2">${result.groundSpeed}</td>
	<td>${result.trueCourse}</td>
	<td>${result.trueHeading}</td>
	<td>${result.magneticHeading}</td>
	<td>${result.distanceTraveled}</td>`),
	toDomObject(
   `<td colspan="2">${result.temp}</td>
	<td>${result.windCorrectionAngle}</td>
	<td>${result.magneticVariation}</td>
	<td></td>
	<td>${result.distanceToGo}</td>`)];
	}
}
