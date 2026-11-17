#pragma once
struct Team {
    const char *id;
    const char *name;
    const char *stadium;
    unsigned int color;
};
inline constexpr Team teams[] = {
    {"ARI", "Arizona Diamondbacks", "Chase Field", 0xA71930FF},
    {"ATL", "Atlanta Braves", "Truist Park", 0xCE1141FF},
    {"BAL", "Baltimore Orioles", "Oriole Park at Camden Yards", 0xDF4601FF},
    {"BOS", "Boston Red Sox", "Fenway Park", 0xBD3039FF},
    {"CHC", "Chicago Cubs", "Wrigley Field", 0x0E3386FF},
    {"CWS", "Chicago White Sox", "Rate Field", 0x27251FFF},
    {"CIN", "Cincinnati Reds", "Great American Ball Park", 0xC6011FFF},
    {"CLE", "Cleveland Guardians", "Progressive Field", 0xE50022FF},
    {"COL", "Colorado Rockies", "Coors Field", 0x663399FF},
    {"DET", "Detroit Tigers", "Comerica Park", 0x0C2340FF},
    {"HOU", "Houston Astros", "Daikin Park", 0xEB6E1FFF},
    {"KC", "Kansas City Royals", "Kauffman Stadium", 0x004687FF},
    {"LAA", "Los Angeles Angels", "Angel Stadium", 0xBA0021FF},
    {"LAD", "Los Angeles Dodgers", "Dodger Stadium", 0x005A9CFF},
    {"MIA", "Miami Marlins", "loanDepot park", 0x00A3E0FF},
    {"MIL", "Milwaukee Brewers", "American Family Field", 0x12284BFF},
    {"MIN", "Minnesota Twins", "Target Field", 0x002B5CFF},
    {"NYM", "New York Mets", "Citi Field", 0xFF5910FF},
    {"NYY", "New York Yankees", "Yankee Stadium", 0x142448FF},
    {"ATH", "Athletics", "Sutter Health Park", 0x006341FF},
    {"PHI", "Philadelphia Phillies", "Citizens Bank Park", 0xE81828FF},
    {"PIT", "Pittsburgh Pirates", "PNC Park", 0xD6A800FF},
    {"SD", "San Diego Padres", "Petco Park", 0x6F482CFF},
    {"SF", "San Francisco Giants", "Oracle Park", 0xFD5A1EFF},
    {"SEA", "Seattle Mariners", "T-Mobile Park", 0x005C5CFF},
    {"STL", "St. Louis Cardinals", "Busch Stadium", 0xC41E3AFF},
    {"TB", "Tampa Bay Rays", "Tropicana Field", 0x092C5CFF},
    {"TEX", "Texas Rangers", "Globe Life Field", 0x003278FF},
    {"TOR", "Toronto Blue Jays", "Rogers Centre", 0x134A8EFF},
    {"WSH", "Washington Nationals", "Nationals Park", 0xAB0003FF},
};
