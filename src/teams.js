// Text metadata only. Artwork and asset mapping are intentionally deferred.
const entries = [
  ['ARI','Arizona Diamondbacks','Chase Field'],['ATL','Atlanta Braves','Truist Park'],
  ['BAL','Baltimore Orioles','Oriole Park at Camden Yards'],['BOS','Boston Red Sox','Fenway Park'],
  ['CHC','Chicago Cubs','Wrigley Field'],['CWS','Chicago White Sox','Rate Field'],
  ['CIN','Cincinnati Reds','Great American Ball Park'],['CLE','Cleveland Guardians','Progressive Field'],
  ['COL','Colorado Rockies','Coors Field'],['DET','Detroit Tigers','Comerica Park'],
  ['HOU','Houston Astros','Daikin Park'],['KC','Kansas City Royals','Kauffman Stadium'],
  ['LAA','Los Angeles Angels','Angel Stadium'],['LAD','Los Angeles Dodgers','Dodger Stadium'],
  ['MIA','Miami Marlins','loanDepot park'],['MIL','Milwaukee Brewers','American Family Field'],
  ['MIN','Minnesota Twins','Target Field'],['NYM','New York Mets','Citi Field'],
  ['NYY','New York Yankees','Yankee Stadium'],['ATH','Athletics','Sutter Health Park'],
  ['PHI','Philadelphia Phillies','Citizens Bank Park'],['PIT','Pittsburgh Pirates','PNC Park'],
  ['SD','San Diego Padres','Petco Park'],['SF','San Francisco Giants','Oracle Park'],
  ['SEA','Seattle Mariners','T-Mobile Park'],['STL','St. Louis Cardinals','Busch Stadium'],
  ['TB','Tampa Bay Rays','Tropicana Field'],['TEX','Texas Rangers','Globe Life Field'],
  ['TOR','Toronto Blue Jays','Rogers Centre'],['WSH','Washington Nationals','Nationals Park']
];
export const teams = entries.map(([id, name, stadium]) => ({ id, name, stadium, assetKey: id.toLowerCase() }));
