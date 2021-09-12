#pragma once
#include <time.h>

#include <string>

struct DateTime
{
    private:
        tm dateTime;
        time_t dateTimeAsTime;

    public:
        DateTime() 
        {
            time( &dateTimeAsTime );
            dateTime = *localtime( &dateTimeAsTime );
        }

        DateTime(const tm& dateTime) : dateTime(dateTime)
        {
            dateTimeAsTime = mktime(&this->dateTime);
        }

        inline int Year() { return dateTime.tm_year + 1900; }

        void AddMinutes(double minutes) 
        {            
            dateTimeAsTime += minutes * 60;
            dateTime = *localtime(&dateTimeAsTime);
        }

        inline double DiffInMinutes(const DateTime& other) const { return (dateTimeAsTime - other.dateTimeAsTime) / 60.0; }

        inline std::string ToString() const 
        {
            char buffer[80] = {};
            strftime(buffer, sizeof(buffer), "%FT%TZ", &dateTime); 
            return std::string(buffer);
        }

        inline bool operator >(const DateTime &b) { return dateTimeAsTime > b.dateTimeAsTime; }
        inline bool operator <(const DateTime &b) { return dateTimeAsTime < b.dateTimeAsTime; }
        inline bool operator ==(const DateTime &b) { return dateTimeAsTime == b.dateTimeAsTime; }
        inline bool operator !=(const DateTime &b) { return dateTimeAsTime == b.dateTimeAsTime; }

        static DateTime Parse(const char* str)
        {
            tm parsedTime;
            strptime(str, "%Y-%m-%dT%H:%M:%S%z", &parsedTime);
            parsedTime.tm_isdst = -1;
            auto workingTime = mktime(&parsedTime);
            workingTime += parsedTime.tm_gmtoff;
            parsedTime = *localtime(&workingTime);

            return DateTime(parsedTime);
        }
};

inline std::ostream& operator <<(std::ostream& os, const DateTime& dt) { return os << dt.ToString(); }