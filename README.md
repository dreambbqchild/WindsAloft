# Winds Aloft

This can combine weather model data that can be extracted with wgrib2, with a database of airport latitude and logitude values to create the wind correction for flight plans between two airports. Not for use as the primary means of calculating this, but it's a good way to double check or grok how ForeFlight et. al. might work under the hood.

When I did my Commercial ASEL checkride, this was one minute over what Foreflight had, my calc by the ole' fashion' way was one minute under so there's a shot this actually works.

On the not for use subject, it would be a bad idea to face this as is on a webserver on the Interent cause it has oppertunities to pown said server for an attacker.
