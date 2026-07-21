#include "TrainerRoster.h"

// Clean factory lambda for rapid, single-line Pokémon generation
// EV Params: HP, ATK, DEF, SPA, SPD, SPE (Max 32 per stat, Max 66 Total)
static EnemyPokemon makePoke(std::string sp, std::string item, std::string ab,
    std::vector<std::string> moves,
    int hp = 0, int atk = 0, int def = 0, int spa = 0, int spd = 0, int spe = 0) {
    EnemyPokemon p;
    p.species = sp;
    p.heldItem = item;
    p.ability = ab;
    p.moves = moves;
    p.evHp = hp; p.evAtk = atk; p.evDef = def; p.evSpA = spa; p.evSpD = spd; p.evSpe = spe;
    return p;
}

// =========================================================================================
// GAUNTLET POOLS
// =========================================================================================

std::vector<TrainerTeam> TrainerRoster::getGauntletTrainers(int stage) {
    std::vector<TrainerTeam> roster;

    if (stage == 1) {
        roster.push_back({ "Joey", "Youngster", "Trainers/trainer001.png", {
            makePoke("Raticate", "None", "Guts", {"quickattack", "hyperfang", "bite", "tailwhip"}),
            makePoke("Pidgeotto", "None", "Keen Eye", {"gust", "quickattack", "sandattack", "roost"}),
            makePoke("Pikachu", "None", "Static", {"thundershock", "quickattack", "tailwhip", "thunderwave"})
        } });
        roster.push_back({ "Bugsy", "Bug Catcher", "Trainers/trainer002.png", {
            makePoke("Butterfree", "None", "Compound Eyes", {"confusion", "gust", "sleeppowder", "poisonpowder"}),
            makePoke("Beedrill", "None", "Swarm", {"twinneedle", "furyattack", "focusenergy", "pursuit"}),
            makePoke("Scyther", "None", "Technician", {"quickattack", "leer", "focusenergy", "wingattack"})
        } });
        roster.push_back({ "Liam", "Camper", "Trainers/trainer003.png", {
            makePoke("Geodude", "None", "Sturdy", {"tackle", "defensecurl", "rockthrow", "magnitude"}),
            makePoke("Mankey", "None", "Vital Spirit", {"scratch", "lowkick", "karatechop", "furyswipes"}),
            makePoke("Growlithe", "None", "Intimidate", {"bite", "roar", "ember", "takedown"})
        } });
        roster.push_back({ "Sarah", "Lass", "Trainers/trainer004.png", {
            makePoke("Clefairy", "None", "Cute Charm", {"pound", "sing", "doubleslap", "minimize"}),
            makePoke("Jigglypuff", "None", "Cute Charm", {"sing", "pound", "defensecurl", "disable"}),
            makePoke("Nidorina", "None", "Poison Point", {"scratch", "tailwhip", "doublekick", "poisonsting"})
        } });
        roster.push_back({ "David", "Hiker", "Trainers/trainer005.png", {
            makePoke("Onix", "None", "Sturdy", {"tackle", "screech", "bind", "rockthrow"}),
            makePoke("Machop", "None", "Guts", {"lowkick", "leer", "focusenergy", "karatechop"}),
            makePoke("Graveler", "None", "Sturdy", {"rockthrow", "magnitude", "selfdestruct", "rollout"})
        } });
        roster.push_back({ "Eugene", "Sailor", "Trainers/trainer006.png", {
            makePoke("Tentacool", "None", "Clear Body", {"poisonsting", "supersonic", "constrict", "acid"}),
            makePoke("Krabby", "None", "Hyper Cutter", {"bubble", "leer", "vicegrip", "mudshot"}),
            makePoke("Poliwhirl", "None", "Water Absorb", {"watergun", "hypnosis", "doubleslap", "bodyslam"})
        } });
    }
    else if (stage == 2) {
        roster.push_back({ "Kenji", "Black Belt", "Trainers/trainer007.png", {
            makePoke("Machoke", "Black Belt", "No Guard", {"crosschop", "knockoff", "bulletpunch", "rockslide"}),
            makePoke("Hitmonlee", "None", "Limber", {"highjumpkick", "machpunch", "blazekick", "fakeout"}),
            makePoke("Hitmonchan", "Punching Glove", "Iron Fist", {"machpunch", "firepunch", "icepunch", "thunderpunch"}),
            makePoke("Primeape", "None", "Defiant", {"closecombat", "uturn", "stoneedge", "punishment"})
        } });
        roster.push_back({ "Yuri", "Psychic", "Trainers/trainer008.png", {
            makePoke("Kadabra", "Twisted Spoon", "Synchronize", {"psychic", "shadowball", "recover", "reflect"}),
            makePoke("Hypno", "Leftovers", "Insomnia", {"hypnosis", "dreameater", "psychic", "shadowball"}),
            makePoke("Slowpoke", "Eviolite", "Regenerator", {"scald", "slackoff", "psychic", "thunderwave"}),
            makePoke("Mr. Mime", "Light Clay", "Filter", {"lightscreen", "reflect", "psychic", "dazzlinggleam"})
        } });
        roster.push_back({ "Isaac", "Scientist", "Trainers/trainer009.png", {
            makePoke("Magneton", "Eviolite", "Magnet Pull", {"thunderbolt", "flashcannon", "voltswitch", "triattack"}),
            makePoke("Electrode", "None", "Static", {"thunderbolt", "voltswitch", "taunt", "explosion"}),
            makePoke("Porygon2", "Eviolite", "Download", {"triattack", "icebeam", "recover", "thunderwave"}),
            makePoke("Weezing", "Black Sludge", "Levitate", {"sludgebomb", "flamethrower", "willowisp", "painsplit"})
        } });
        roster.push_back({ "Lance", "Tamer", "Trainers/trainer010.png", {
            makePoke("Dragonair", "Eviolite", "Shed Skin", {"dragondance", "outrage", "waterfall", "extremespeed"}),
            makePoke("Gyarados", "Mystic Water", "Intimidate", {"waterfall", "bite", "dragondance", "icefang"}),
            makePoke("Aerodactyl", "Hard Stone", "Rock Head", {"rockslide", "wingattack", "bite", "roost"}),
            makePoke("Arbok", "Poison Barb", "Intimidate", {"gunkshot", "suckerpunch", "glare", "earthquake"})
        } });
        roster.push_back({ "Alice", "Hex Maniac", "Trainers/trainer011.png", {
            makePoke("Haunter", "Eviolite", "Levitate", {"shadowball", "sludgebomb", "destinybond", "willowisp"}),
            makePoke("Misdreavus", "Eviolite", "Levitate", {"shadowball", "painsplit", "willowisp", "taunt"}),
            makePoke("Marowak-Alola", "Thick Club", "Lightning Rod", {"shadowbone", "flareblitz", "bonemerang", "willowisp"}),
            makePoke("Gengar", "Spell Tag", "Cursed Body", {"shadowball", "sludgewave", "focusblast", "destinybond"})
        } });
        roster.push_back({ "Blue", "Ace Trainer", "Trainers/trainer012.png", {
            makePoke("Charmeleon", "Eviolite", "Blaze", {"flamethrower", "dragonrage", "smokescreen", "scaryface"}),
            makePoke("Ivysaur", "Eviolite", "Overgrow", {"razorleaf", "sleeppowder", "leechseed", "sludgebomb"}),
            makePoke("Wartortle", "Eviolite", "Torrent", {"waterpulse", "bite", "rapidspin", "protect"}),
            makePoke("Snorlax", "Leftovers", "Thick Fat", {"bodyslam", "crunch", "rest", "sleeptalk"})
        } });
    }
    else if (stage == 3) {
        roster.push_back({ "Rain", "Weather Setter", "Trainers/trainer013.png", {
            makePoke("Pelipper", "Damp Rock", "Drizzle", {"scald", "hurricane", "roost", "uturn"}, 32, 0, 32, 0, 2, 0),
            makePoke("Ludicolo", "Life Orb", "Swift Swim", {"hydropump", "gigadrain", "icebeam", "raindance"}, 0, 0, 0, 32, 2, 32),
            makePoke("Kingdra", "Choice Specs", "Swift Swim", {"hydropump", "dracometeor", "surf", "icebeam"}, 0, 0, 0, 32, 2, 32),
            makePoke("Raichu-Alola", "Aloraichium Z", "Surge Surfer", {"thunderbolt", "psychic", "focusblast", "voltswitch"}, 0, 0, 0, 32, 2, 32),
            makePoke("Toxicroak", "Life Orb", "Dry Skin", {"swordsdance", "gunkshot", "drainpunch", "suckerpunch"}, 0, 32, 0, 0, 2, 32)
        } });
        roster.push_back({ "Sun", "Weather Setter", "Trainers/trainer014.png", {
            makePoke("Torkoal", "Heat Rock", "Drought", {"lavaplume", "earthpower", "stealthrock", "rapidspin"}, 32, 0, 32, 0, 2, 0),
            makePoke("Venusaur", "Life Orb", "Chlorophyll", {"gigadrain", "sludgebomb", "weatherball", "growth"}, 0, 0, 0, 32, 2, 32),
            makePoke("Houndoom", "Life Orb", "Solar Power", {"fireblast", "darkpulse", "nastyplot", "sludgebomb"}, 0, 0, 0, 32, 2, 32),
            makePoke("Shiftry", "Focus Sash", "Chlorophyll", {"knockoff", "leafblade", "suckerpunch", "swordsdance"}, 0, 32, 0, 0, 2, 32),
            makePoke("Arcanine", "Heavy-Duty Boots", "Intimidate", {"flareblitz", "morningsun", "willowisp", "teleport"}, 32, 0, 32, 0, 2, 0)
        } });
        roster.push_back({ "Dusty", "Sandstorm Crew", "Trainers/trainer015.png", {
            makePoke("Hippowdon", "Smooth Rock", "Sand Stream", {"earthquake", "slackoff", "stealthrock", "whirlwind"}, 32, 0, 32, 0, 2, 0),
            makePoke("Excadrill", "Life Orb", "Sand Rush", {"earthquake", "ironhead", "rockslide", "swordsdance"}, 0, 32, 0, 0, 2, 32),
            makePoke("Stoutland", "Choice Band", "Sand Rush", {"return", "crunch", "playrough", "superpower"}, 0, 32, 0, 0, 2, 32),
            makePoke("Tyranitar", "Choice Band", "Sand Stream", {"stoneedge", "crunch", "pursuit", "earthquake"}, 0, 32, 0, 0, 2, 32),
            makePoke("Clefable", "Leftovers", "Magic Guard", {"moonblast", "softboiled", "stealthrock", "fireblast"}, 32, 0, 32, 0, 2, 0)
        } });
        roster.push_back({ "Dizzy", "Trick Room Team", "Trainers/trainer016.png", {
            makePoke("Reuniclus", "Life Orb", "Magic Guard", {"trickroom", "psychic", "focusblast", "recover"}, 32, 0, 0, 32, 2, 0),
            makePoke("Marowak-Alola", "Thick Club", "Lightning Rod", {"flareblitz", "shadowbone", "bonemerang", "swordsdance"}, 32, 32, 0, 0, 2, 0),
            makePoke("Snorlax", "Leftovers", "Thick Fat", {"curse", "bodyslam", "earthquake", "rest"}, 32, 0, 2, 0, 32, 0),
            makePoke("Slowbro", "Leftovers", "Regenerator", {"scald", "slackoff", "trickroom", "teleport"}, 32, 0, 32, 0, 2, 0),
            makePoke("Escavalier", "Choice Band", "Overcoat", {"megahorn", "ironhead", "knockoff", "drillrun"}, 32, 32, 0, 0, 2, 0)
        } });
        roster.push_back({ "Spike", "Hazard Stack", "Trainers/trainer017.png", {
            makePoke("Skarmory", "Rocky Helmet", "Sturdy", {"spikes", "roost", "bravebird", "whirlwind"}, 32, 0, 32, 0, 2, 0),
            makePoke("Forretress", "Leftovers", "Sturdy", {"toxicspikes", "rapidspin", "voltturn", "gyroball"}, 32, 0, 32, 0, 2, 0),
            makePoke("Gengar", "Focus Sash", "Cursed Body", {"shadowball", "sludgewave", "taunt", "destinybond"}, 0, 0, 0, 32, 2, 32),
            makePoke("Bisharp", "Black Glasses", "Defiant", {"knockoff", "ironhead", "suckerpunch", "swordsdance"}, 0, 32, 0, 0, 2, 32),
            makePoke("Azumarill", "Choice Band", "Huge Power", {"liquidation", "playrough", "aquajet", "superpower"}, 32, 32, 0, 0, 2, 0)
        } });
        roster.push_back({ "Vance", "Veteran", "Trainers/trainer018.png", {
            makePoke("Venusaur", "Black Sludge", "Overgrow", {"gigadrain", "sludgebomb", "leechseed", "protect"}, 32, 0, 2, 32, 0, 0),
            makePoke("Charizard", "Heavy-Duty Boots", "Blaze", {"fireblast", "hurricane", "roost", "defog"}, 0, 0, 0, 32, 2, 32),
            makePoke("Blastoise", "Leftovers", "Torrent", {"scald", "rapidspin", "toxic", "protect"}, 32, 0, 32, 0, 2, 0),
            makePoke("Nidoking", "Life Orb", "Sheer Force", {"sludgewave", "earthpower", "icebeam", "flamethrower"}, 0, 0, 0, 32, 2, 32),
            makePoke("Nidoqueen", "Black Sludge", "Sheer Force", {"stealthrock", "earthpower", "sludgewave", "toxic"}, 32, 0, 32, 0, 2, 0)
        } });
    }
    else if (stage >= 4) {
        roster.push_back({ "Ruby", "Hyper Offense", "Trainers/trainer019.png", {
            makePoke("Garchomp", "Focus Sash", "Rough Skin", {"stealthrock", "earthquake", "outrage", "swordsdance"}, 0, 32, 0, 0, 2, 32),
            makePoke("Scizor", "Choice Band", "Technician", {"bulletpunch", "uturn", "knockoff", "superpower"}, 32, 32, 0, 0, 2, 0),
            makePoke("Volcarona", "Heavy-Duty Boots", "Flame Body", {"quiverdance", "fierydance", "bugbuzz", "roost"}, 0, 0, 0, 32, 2, 32),
            makePoke("Greninja", "Life Orb", "Protean", {"hydropump", "darkpulse", "icebeam", "gunkshot"}, 0, 2, 0, 32, 0, 32),
            makePoke("Serperior", "Leftovers", "Contrary", {"leafstorm", "hiddenpowerfire", "substitute", "glare"}, 0, 0, 0, 32, 2, 32),
            makePoke("Mimikyu", "Life Orb", "Disguise", {"swordsdance", "playrough", "shadowclaw", "shadowsneak"}, 0, 32, 0, 0, 2, 32)
        } });
        roster.push_back({ "Sapphire", "Bulky Offense", "Trainers/trainer020.png", {
            makePoke("Tyranitar", "Choice Band", "Sand Stream", {"stoneedge", "crunch", "pursuit", "earthquake"}, 0, 32, 0, 0, 2, 32),
            makePoke("Excadrill", "Air Balloon", "Sand Rush", {"earthquake", "ironhead", "rapidspin", "swordsdance"}, 0, 32, 0, 0, 2, 32),
            makePoke("Togekiss", "Leftovers", "Serene Grace", {"airslash", "nastyplot", "roost", "healbell"}, 32, 0, 0, 2, 32, 0),
            makePoke("Slowbro", "Rocky Helmet", "Regenerator", {"scald", "slackoff", "icebeam", "teleport"}, 32, 0, 32, 0, 2, 0),
            makePoke("Amoonguss", "Black Sludge", "Regenerator", {"spore", "gigadrain", "sludgebomb", "clearsmog"}, 32, 0, 17, 0, 17, 0),
            makePoke("Cinderace", "Heavy-Duty Boots", "Libero", {"pyroball", "highjumpkick", "uturn", "suckerpunch"}, 0, 32, 0, 0, 2, 32)
        } });
        roster.push_back({ "Aqua", "Rain Offense", "Trainers/trainer021.png", {
            makePoke("Pelipper", "Damp Rock", "Drizzle", {"scald", "hurricane", "roost", "uturn"}, 32, 0, 32, 0, 2, 0),
            makePoke("Swampert", "Leftovers", "Torrent", {"stealthrock", "earthquake", "flipturn", "scald"}, 32, 0, 32, 0, 2, 0),
            makePoke("Ferrothorn", "Leftovers", "Iron Barbs", {"spikes", "leechseed", "powerwhip", "knockoff"}, 32, 0, 2, 0, 32, 0),
            makePoke("Kingdra", "Choice Specs", "Swift Swim", {"hydropump", "dracometeor", "surf", "icebeam"}, 0, 0, 0, 32, 2, 32),
            makePoke("Hawlucha", "Electric Seed", "Unburden", {"swordsdance", "closecombat", "acrobatics", "roost"}, 0, 32, 0, 0, 2, 32),
            makePoke("Azumarill", "Choice Band", "Huge Power", {"liquidation", "playrough", "aquajet", "superpower"}, 32, 32, 0, 0, 2, 0)
        } });
        roster.push_back({ "Magma", "Sun Offense", "Trainers/trainer022.png", {
            makePoke("Ninetales", "Heat Rock", "Drought", {"fireblast", "solarbeam", "willowisp", "nastyplot"}, 0, 0, 0, 32, 2, 32),
            makePoke("Charizard", "Heavy-Duty Boots", "Solar Power", {"fireblast", "solarbeam", "focusblast", "roost"}, 0, 0, 0, 32, 2, 32),
            makePoke("Venusaur", "Life Orb", "Chlorophyll", {"gigadrain", "sludgebomb", "weatherball", "growth"}, 0, 0, 0, 32, 2, 32),
            makePoke("Darmanitan", "Choice Band", "Sheer Force", {"flareblitz", "rockslide", "earthquake", "uturn"}, 0, 32, 0, 0, 2, 32),
            makePoke("Mandibuzz", "Heavy-Duty Boots", "Overcoat", {"foulplay", "roost", "defog", "uturn"}, 32, 0, 32, 0, 2, 0),
            makePoke("Donphan", "Leftovers", "Sturdy", {"earthquake", "iceshard", "rapidspin", "stealthrock"}, 32, 32, 0, 0, 2, 0)
        } });
        roster.push_back({ "Spark", "Volt-Turn Core", "Trainers/trainer023.png", {
            makePoke("Rotom-Wash", "Leftovers", "Levitate", {"voltswitch", "hydropump", "willowisp", "painsplit"}, 32, 0, 32, 0, 2, 0),
            makePoke("Scizor", "Choice Band", "Technician", {"bulletpunch", "uturn", "knockoff", "superpower"}, 32, 32, 0, 0, 2, 0),
            makePoke("Infernape", "Life Orb", "Iron Fist", {"flareblitz", "closecombat", "machpunch", "uturn"}, 0, 32, 0, 0, 2, 32),
            makePoke("Corviknight", "Leftovers", "Pressure", {"bravebird", "roost", "defog", "uturn"}, 32, 0, 32, 0, 2, 0),
            makePoke("Amoonguss", "Black Sludge", "Regenerator", {"spore", "gigadrain", "sludgebomb", "clearsmog"}, 32, 0, 17, 0, 17, 0),
            makePoke("Mamoswine", "Life Orb", "Thick Fat", {"earthquake", "iciclecrash", "iceshard", "stealthrock"}, 0, 32, 0, 0, 2, 32)
        } });
        roster.push_back({ "Phantom", "Balanced Stall", "Trainers/trainer024.png", {
            makePoke("Toxapex", "Black Sludge", "Regenerator", {"scald", "recover", "haze", "toxicspikes"}, 32, 0, 32, 0, 2, 0),
            makePoke("Clefable", "Leftovers", "Magic Guard", {"moonblast", "softboiled", "stealthrock", "flamethrower"}, 32, 0, 32, 0, 2, 0),
            makePoke("Skarmory", "Rocky Helmet", "Sturdy", {"spikes", "roost", "bravebird", "whirlwind"}, 32, 0, 32, 0, 2, 0),
            makePoke("Blissey", "Heavy-Duty Boots", "Natural Cure", {"seismictoss", "softboiled", "toxic", "teleport"}, 32, 0, 32, 0, 2, 0),
            makePoke("Quagsire", "Leftovers", "Unaware", {"scald", "earthquake", "recover", "toxic"}, 32, 0, 32, 0, 2, 0),
            makePoke("Sableye", "Leftovers", "Prankster", {"knockoff", "willowisp", "recover", "taunt"}, 32, 0, 32, 0, 2, 0)
        } });
    }

    return roster;
}

// =========================================================================================
// TOURNAMENT POOLS
// =========================================================================================

std::vector<TrainerTeam> TrainerRoster::getTournamentGymLeaders() {
    std::vector<TrainerTeam> gymLeaders;

    gymLeaders.push_back({ "Misty", "Gym Leader", "Trainers/trainer030.png", {
        makePoke("Pelipper", "Damp Rock", "Drizzle", {"scald", "hurricane", "roost", "uturn"}, 32, 0, 32, 0, 2, 0),
        makePoke("Swampert", "Swampertite", "Torrent", {"waterfall", "earthquake", "flipturn", "icepunch"}, 0, 32, 0, 0, 2, 32),
        makePoke("Palafin", "Mystic Water", "Zero to Hero", {"jetpunch", "wavecrash", "closecombat", "flipturn"}, 0, 32, 0, 0, 2, 32),
        makePoke("Urshifu-Rapid-Strike", "Choice Band", "Unseen Fist", {"surgingstrikes", "closecombat", "aquajet", "uturn"}, 0, 32, 0, 0, 2, 32),
        makePoke("Toxapex", "Black Sludge", "Regenerator", {"scald", "toxic", "recover", "haze"}, 32, 0, 32, 0, 2, 0),
        makePoke("Manaphy", "Leftovers", "Hydration", {"tailglow", "surf", "icebeam", "energyball"}, 0, 0, 0, 32, 2, 32)
    } });

    gymLeaders.push_back({ "Blaine", "Gym Leader", "Trainers/trainer031.png", {
        makePoke("Torkoal", "Heat Rock", "Drought", {"lavaplume", "stealthrock", "rapidspin", "yawn"}, 32, 0, 32, 0, 2, 0),
        makePoke("Charizard", "Charizardite Y", "Blaze", {"fireblast", "solarbeam", "focusblast", "roost"}, 0, 0, 0, 32, 2, 32),
        makePoke("Chi-Yu", "Choice Specs", "Beads of Ruin", {"overheat", "darkpulse", "flamethrower", "psychic"}, 0, 0, 0, 32, 2, 32),
        makePoke("Cinderace", "Heavy-Duty Boots", "Libero", {"pyroball", "highjumpkick", "uturn", "suckerpunch"}, 0, 32, 0, 0, 2, 32),
        makePoke("Volcarona", "Heavy-Duty Boots", "Flame Body", {"quiverdance", "fierydance", "bugbuzz", "gigadrain"}, 0, 0, 0, 32, 2, 32),
        makePoke("Heatran", "Air Balloon", "Flash Fire", {"magmastorm", "earthpower", "taunt", "toxic"}, 32, 0, 0, 32, 2, 0)
    } });

    gymLeaders.push_back({ "Erika", "Gym Leader", "Trainers/trainer032.png", {
        makePoke("Venusaur", "Venusaurite", "Overgrow", {"gigadrain", "sludgebomb", "synthesis", "leechseed"}, 32, 0, 2, 32, 0, 0),
        makePoke("Rillaboom", "Choice Band", "Grassy Surge", {"woodhammer", "grassyglide", "uturn", "knockoff"}, 0, 32, 0, 0, 2, 32),
        makePoke("Ferrothorn", "Leftovers", "Iron Barbs", {"powerwhip", "gyroball", "stealthrock", "leechseed"}, 32, 0, 2, 0, 32, 0),
        makePoke("Ogerpon-Wellspring", "Wellspring Mask", "Water Absorb", {"ivycudgel", "hornleech", "swordsdance", "spikyshield"}, 0, 32, 0, 0, 2, 32),
        makePoke("Kartana", "Choice Scarf", "Beast Boost", {"leafblade", "smartstrike", "sacredsword", "knockoff"}, 0, 32, 0, 0, 2, 32),
        makePoke("Amoonguss", "Black Sludge", "Regenerator", {"spore", "gigadrain", "sludgebomb", "clearsmog"}, 32, 0, 17, 0, 17, 0)
    } });

    gymLeaders.push_back({ "Surge", "Gym Leader", "Trainers/trainer033.png", {
        makePoke("Tapu Koko", "Light Clay", "Electric Surge", {"thunderbolt", "dazzlinggleam", "reflect", "lightscreen"}, 0, 0, 0, 32, 2, 32),
        makePoke("Raichu-Alola", "Life Orb", "Surge Surfer", {"risingvoltage", "psychic", "focusblast", "surf"}, 0, 0, 0, 32, 2, 32),
        makePoke("Iron Hands", "Assault Vest", "Quark Drive", {"drainpunch", "thunderpunch", "fakeout", "icepunch"}, 32, 32, 0, 0, 2, 0),
        makePoke("Rotom-Wash", "Leftovers", "Levitate", {"hydropump", "voltswitch", "willowisp", "painsplit"}, 32, 0, 32, 0, 2, 0),
        makePoke("Zapdos", "Heavy-Duty Boots", "Static", {"discharge", "hurricane", "roost", "heatwave"}, 32, 0, 32, 0, 2, 0),
        makePoke("Magnezone", "Choice Specs", "Magnet Pull", {"thunderbolt", "flashcannon", "voltswitch", "triattack"}, 0, 0, 0, 32, 2, 32)
    } });

    gymLeaders.push_back({ "Valerie", "Gym Leader", "Trainers/trainer034.png", {
        makePoke("Tapu Lele", "Choice Specs", "Psychic Surge", {"psychic", "moonblast", "shadowball", "focusblast"}, 0, 0, 0, 32, 2, 32),
        makePoke("Clefable", "Leftovers", "Unaware", {"moonblast", "softboiled", "flamethrower", "stealthrock"}, 32, 0, 32, 0, 2, 0),
        makePoke("Iron Valiant", "Booster Energy", "Quark Drive", {"moonblast", "closecombat", "knockoff", "shadowsneak"}, 0, 32, 0, 0, 2, 32),
        makePoke("Magearna", "Assault Vest", "Soul-Heart", {"fleurcannon", "flashcannon", "voltswitch", "aurasphere"}, 32, 0, 0, 32, 2, 0),
        makePoke("Hatterene", "Leftovers", "Magic Bounce", {"calmmind", "drainingkiss", "psyshock", "mysticalfire"}, 32, 0, 2, 32, 0, 0),
        makePoke("Diancie", "Diancite", "Clear Body", {"diamondstorm", "moonblast", "earthpower", "protect"}, 0, 32, 0, 32, 0, 2)
    } });

    gymLeaders.push_back({ "Giovanni", "Gym Leader", "Trainers/trainer035.png", {
        makePoke("Hippowdon", "Smooth Rock", "Sand Stream", {"earthquake", "slackoff", "stealthrock", "whirlwind"}, 32, 0, 32, 0, 2, 0),
        makePoke("Excadrill", "Air Balloon", "Sand Rush", {"earthquake", "ironhead", "rapidspin", "swordsdance"}, 0, 32, 0, 0, 2, 32),
        makePoke("Landorus-Therian", "Choice Scarf", "Intimidate", {"earthquake", "uturn", "stoneedge", "defog"}, 0, 32, 0, 0, 2, 32),
        makePoke("Great Tusk", "Booster Energy", "Protosynthesis", {"earthquake", "closecombat", "knockoff", "rapidspin"}, 0, 32, 2, 0, 0, 32),
        makePoke("Gliscor", "Toxic Orb", "Poison Heal", {"earthquake", "roost", "toxic", "protect"}, 32, 0, 32, 0, 2, 0),
        makePoke("Garchomp", "Garchompite", "Rough Skin", {"earthquake", "outrage", "swordsdance", "firefang"}, 0, 32, 0, 0, 2, 32)
    } });

    gymLeaders.push_back({ "Morty", "Gym Leader", "Trainers/trainer036.png", {
        makePoke("Gholdengo", "Choice Specs", "Good as Gold", {"makeitrain", "shadowball", "trick", "focusblast"}, 0, 0, 0, 32, 2, 32),
        makePoke("Dragapult", "Choice Band", "Infiltrator", {"dragondarts", "phantomforce", "uturn", "suckerpunch"}, 0, 32, 0, 0, 2, 32),
        makePoke("Gengar", "Gengarite", "Cursed Body", {"shadowball", "sludgewave", "focusblast", "destinybond"}, 0, 0, 0, 32, 2, 32),
        makePoke("Flutter Mane", "Booster Energy", "Protosynthesis", {"shadowball", "moonblast", "mysticalfire", "psyshock"}, 0, 0, 0, 32, 2, 32),
        makePoke("Skeledirge", "Heavy-Duty Boots", "Unaware", {"torchsong", "shadowball", "slackoff", "willowisp"}, 32, 0, 32, 0, 2, 0),
        makePoke("Aegislash", "Leftovers", "Stance Change", {"shadowball", "kingsshield", "flashcannon", "shadowsneak"}, 32, 0, 0, 32, 2, 0)
    } });

    gymLeaders.push_back({ "Jasmine", "Gym Leader", "Trainers/trainer037.png", {
        makePoke("Skarmory", "Rocky Helmet", "Sturdy", {"spikes", "roost", "bravebird", "whirlwind"}, 32, 0, 32, 0, 2, 0),
        makePoke("Scizor", "Scizorite", "Technician", {"bulletpunch", "uturn", "swordsdance", "roost"}, 32, 32, 0, 0, 2, 0),
        makePoke("Gholdengo", "Air Balloon", "Good as Gold", {"makeitrain", "shadowball", "recover", "nastyplot"}, 0, 0, 0, 32, 2, 32),
        makePoke("Heatran", "Leftovers", "Flash Fire", {"magmastorm", "earthpower", "taunt", "stealthrock"}, 32, 0, 0, 32, 2, 0),
        makePoke("Kingambit", "Black Glasses", "Supreme Overlord", {"kowtowcleave", "ironhead", "suckerpunch", "swordsdance"}, 32, 32, 0, 0, 2, 0),
        makePoke("Corviknight", "Leftovers", "Pressure", {"bravebird", "bodypress", "roost", "defog"}, 32, 0, 32, 0, 2, 0)
    } });

    return gymLeaders;
}

std::vector<TrainerTeam> TrainerRoster::getTournamentEliteFour() {
    std::vector<TrainerTeam> eliteFour;

    eliteFour.push_back({ "Lance", "Elite Four", "Trainers/trainer040.png", {
        makePoke("Salamence", "Salamencite", "Intimidate", {"doubleedge", "dragondance", "roost", "earthquake"}, 0, 32, 0, 0, 2, 32),
        makePoke("Garchomp", "Focus Sash", "Rough Skin", {"earthquake", "outrage", "swordsdance", "stealthrock"}, 0, 32, 0, 0, 2, 32),
        makePoke("Roaring Moon", "Booster Energy", "Protosynthesis", {"knockoff", "dragonclaw", "dragondance", "acrobatics"}, 0, 32, 0, 0, 2, 32),
        makePoke("Dragapult", "Choice Specs", "Infiltrator", {"dracometeor", "shadowball", "flamethrower", "uturn"}, 0, 0, 0, 32, 2, 32),
        makePoke("Gholdengo", "Leftovers", "Good as Gold", {"makeitrain", "shadowball", "recover", "nastyplot"}, 0, 0, 0, 32, 2, 32),
        makePoke("Toxapex", "Black Sludge", "Regenerator", {"scald", "toxic", "recover", "haze"}, 32, 0, 32, 0, 2, 0)
    } });

    eliteFour.push_back({ "Grimsley", "Elite Four", "Trainers/trainer041.png", {
        makePoke("Kingambit", "Black Glasses", "Supreme Overlord", {"kowtowcleave", "ironhead", "suckerpunch", "swordsdance"}, 32, 32, 0, 0, 2, 0),
        makePoke("Darkrai", "Life Orb", "Bad Dreams", {"darkpulse", "icebeam", "focusblast", "nastyplot"}, 0, 0, 0, 32, 2, 32),
        makePoke("Ting-Lu", "Leftovers", "Vessel of Ruin", {"earthquake", "ruination", "stealthrock", "whirlwind"}, 32, 0, 32, 0, 2, 0),
        makePoke("Roaring Moon", "Booster Energy", "Protosynthesis", {"knockoff", "dragonclaw", "dragondance", "taunt"}, 0, 32, 0, 0, 2, 32),
        makePoke("Gliscor", "Toxic Orb", "Poison Heal", {"earthquake", "roost", "toxic", "protect"}, 32, 0, 32, 0, 2, 0),
        makePoke("Toxapex", "Black Sludge", "Regenerator", {"scald", "banefulbunker", "recover", "toxicspikes"}, 32, 0, 32, 0, 2, 0)
    } });

    eliteFour.push_back({ "Caitlin", "Elite Four", "Trainers/trainer042.png", {
        makePoke("Tapu Lele", "Choice Specs", "Psychic Surge", {"psychic", "moonblast", "focusblast", "shadowball"}, 0, 0, 0, 32, 2, 32),
        makePoke("Mewtwo", "Mewtwonite Y", "Pressure", {"psystrike", "icebeam", "fireblast", "aurasphere"}, 0, 0, 0, 32, 2, 32),
        makePoke("Hatterene", "Leftovers", "Magic Bounce", {"calmmind", "drainingkiss", "psyshock", "mysticalfire"}, 32, 0, 2, 32, 0, 0),
        makePoke("Deoxys-Attack", "Focus Sash", "Pressure", {"psychoboost", "knockoff", "superpower", "icebeam"}, 0, 32, 0, 32, 0, 2),
        makePoke("Great Tusk", "Booster Energy", "Protosynthesis", {"earthquake", "closecombat", "knockoff", "rapidspin"}, 0, 32, 2, 0, 0, 32),
        makePoke("Corviknight", "Rocky Helmet", "Pressure", {"bravebird", "bodypress", "roost", "defog"}, 32, 0, 32, 0, 2, 0)
    } });

    eliteFour.push_back({ "Koga", "Elite Four", "Trainers/trainer043.png", {
        makePoke("Toxapex", "Black Sludge", "Regenerator", {"scald", "toxic", "recover", "haze"}, 32, 0, 32, 0, 2, 0),
        makePoke("Eternatus", "Black Sludge", "Pressure", {"dynamaxcannon", "sludgewave", "flamethrower", "recover"}, 32, 0, 0, 32, 2, 0),
        makePoke("Sneasler", "White Herb", "Unburden", {"direclaw", "closecombat", "shadowclaw", "swordsdance"}, 0, 32, 0, 0, 2, 32),
        makePoke("Glimmora", "Focus Sash", "Toxic Debris", {"mortalspin", "sludgewave", "earthpower", "stealthrock"}, 0, 0, 0, 32, 2, 32),
        makePoke("Ting-Lu", "Leftovers", "Vessel of Ruin", {"earthquake", "ruination", "whirlwind", "protect"}, 32, 0, 32, 0, 2, 0),
        makePoke("Corviknight", "Leftovers", "Pressure", {"bravebird", "roost", "defog", "bodypress"}, 32, 0, 32, 0, 2, 0)
    } });

    return eliteFour;
}

TrainerTeam TrainerRoster::getTournamentChampion() {
    return { "Cynthia", "Champion", "Trainers/trainer050.png", {
        makePoke("Garchomp", "Garchompite", "Rough Skin", {"earthquake", "outrage", "swordsdance", "stoneedge"}, 0, 32, 0, 0, 2, 32),
        makePoke("Groudon", "Red Orb", "Drought", {"precipiceblades", "firepunch", "stealthrock", "swordsdance"}, 32, 32, 0, 0, 2, 0),
        makePoke("Zacian-Crowned", "Rusted Sword", "Intrepid Sword", {"behemothblade", "playrough", "closecombat", "wildcharge"}, 0, 32, 0, 0, 2, 32),
        makePoke("Yveltal", "Life Orb", "Dark Aura", {"darkpulse", "oblivionwing", "suckerpunch", "taunt"}, 0, 0, 0, 32, 2, 32),
        makePoke("Necrozma-Dusk-Mane", "Weakness Policy", "Prism Armor", {"sunsteelstrike", "photongeyser", "earthquake", "dragondance"}, 32, 32, 0, 0, 2, 0),
        makePoke("Kyogre", "Blue Orb", "Drizzle", {"hydropump", "icebeam", "thunder", "calmmind"}, 32, 0, 0, 32, 2, 0)
    } };
}


TrainerTeam TrainerRoster::getSecretBoss() {
    return { "Gabi Mircea", "Immortal", "Trainers/trainer_gabi.png", {
        makePoke("Rayquaza", "Life Orb", "Air Lock", {"dragonascent", "vcreate", "extremespeed", "swordsdance"}, 0, 32, 0, 0, 2, 32),
        makePoke("Eternatus", "Black Sludge", "Pressure", {"dynamaxcannon", "recover", "sludgewave", "flamethrower"}, 32, 0, 32, 0, 2, 0),
        makePoke("Calyrex-Shadow", "Focus Sash", "As One (Spectrier)", {"astralbarrage", "psyshock", "nastyplot", "drainingkiss"}, 0, 0, 0, 32, 2, 32),
        makePoke("Zacian-Crowned", "Rusted Sword", "Intrepid Sword", {"behemothblade", "playrough", "closecombat", "swordsdance"}, 0, 32, 0, 0, 2, 32),
        makePoke("Arceus", "Silk Scarf", "Multitype", {"extremespeed", "swordsdance", "shadowclaw", "earthquake"}, 32, 32, 0, 0, 2, 0),
        makePoke("Ditto", "Choice Scarf", "Imposter", {"transform"}, 32, 0, 32, 0, 32, 0)
    } };
}