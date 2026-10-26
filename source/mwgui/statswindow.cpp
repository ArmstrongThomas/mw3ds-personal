// The character: journal, stats, character creation, level progress, death and the end
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include "mwgui/windowmanagerimp.hpp"
#include "mwbase/session.hpp"

std::string statsText(Session& ss, bool progress)
{
	World& w = ss.w;
	const PlayerStats& s = w.stats;
	std::string race, cls, sign;
	for (auto& r : w.game.races)
		if (lower(r.id) == lower(s.race)) race = r.name;
	const ClassDef* c = w.playerClass();
	if (c) cls = c->name;
	if (const BirthDef* b = w.birthsign()) sign = b->name;
	char buf[128];
	const GameData& g = w.game;
	std::string t = g.gmstLabel("sname", "Name") + ": " + s.name + "\n" + g.gmstLabel("srace", "Race") + ": " + race +
		(s.female ? " (female)" : " (male)") + "\n" + g.gmstLabel("sclass", "Class") + ": " + cls + "\n" +
		g.gmstLabel("sbirthsign", "Sign") + ": " + sign +
		(progress ? "" : "\n" + g.gmstLabel("slevel", "Level") + ": " + std::to_string(s.level)) + "\n\n";
	snprintf(buf, sizeof(buf), "%s %d   %s %d   %s %d\n\n", g.gmstLabel("shealth", "Health").c_str(), (int)s.healthMax,
		g.gmstLabel("smagic", "Magicka").c_str(), (int)s.magickaMax, g.gmstLabel("sfatigue", "Fatigue").c_str(), (int)s.fatigueMax);
	t += buf;
	for (int i = 0; i < 8; i++)
		t += attrName(w, i) + ": " + std::to_string(s.attributes[i]) + (i % 2 ? "\n" : "	 ");
	auto section = [&](const std::string& title, auto pred) {
		t += std::string("\n") + title + "\n";
		for (int k = 0; k < 27; k++)
			if (pred(k))
			{
				t += "  " + skillName(w, k) + ": " + std::to_string(s.skills[k]);
				// Progress towards the next point
				if (progress && s.skills[k] < 100)
					t += "  (" + std::to_string((int)(100.0f * s.skillProgress[k] / ss.skillNeed(k))) + "%)";
				t += "\n";
			}
	};
	auto inList = [](const int* arr, int k) { for (int i = 0; i < 5; i++) if (arr[i] == k) return true; return false; };
	if (c)
	{
		section(w.game.gmstLabel("sskillclassmajor", "Major Skills"), [&](int k) { return inList(c->major, k); });
		section(w.game.gmstLabel("sskillclassminor", "Minor Skills"), [&](int k) { return inList(c->minor, k); });
		section(w.game.gmstLabel("sskillclassmisc", "Misc Skills"), [&](int k) { return !inList(c->major, k) && !inList(c->minor, k); });
	}
	// Factions (rank, reputation there), reputation, bounty
	if (!w.pcRank.empty())
	{
		t += "\nFactions\n";
		for (auto& f : w.pcRank)
		{
			t += "  " + w.game.factionName(f.first) + ": " + w.game.rankName(f.first, f.second);
			auto rep = w.pcFacRep.find(f.first);
			if (rep != w.pcFacRep.end() && rep->second)
				t += "  (reputation " + std::to_string(rep->second) + ")";
			if (w.pcExpelled.count(f.first))
				t += "  - expelled";
			t += "\n";
		}
	}
	t += "\n" + w.game.gmstLabel("sreputation", "Reputation") + ": " + std::to_string(w.pcReputation) + "   " +
		w.game.gmstLabel("sbounty", "Bounty") + ": " + std::to_string(w.bounty) + "\n";
	return t;
}

static std::string statsFit(std::string text, float width, float scale)
{
	if (uiTextWidth(text, scale) <= width) return text;
	while (!text.empty() && uiTextWidth(text + "...", scale) > width)
	{
		while (!text.empty() && ((u8)text.back() & 0xC0) == 0x80) text.pop_back();
		if (!text.empty()) text.pop_back();
	}
	return text + "...";
}

void Session::drawStats()
{
	const UiInput& in = uiIn();
	const PlayerStats& s = w.stats;
	const GameData& g = w.game;
	const ClassDef* cls = w.playerClass();
	header(g.gmstLabel("sstats", "Stats"), s.name);
	int previousTab = statsTab;
	if (in.down & KEY_L) statsTab = (statsTab + 3) % 4;
	if (in.down & KEY_R) statsTab = (statsTab + 1) % 4;
	const char* tabs[] = { "Character", "Attributes", "Skills", "Factions" };
	for (int i = 0; i < 4; i++)
		if (uiButton(4 + i * 79, 24, 75, 24, tabs[i], statsTab == i)) statsTab = i;
	if (statsTab != previousTab)
	{
		statsSkillPage = statsFactionPage = 0;
		playSound(-1, "Menu Click");
	}
	uiPanel(4, 52, 312, 152);
	statsRowsShown = 0;
	statsPageCount = 1;
	auto fact = [&](float y, const std::string& label, const std::string& value) {
		uiText(12, y, 0.42f, col::textDim, label);
		uiText(100, y, 0.45f, col::header, statsFit(value, 204, 0.45f));
	};
	auto pager = [&](int& page, int count, int perPage) {
		statsPageCount = std::max(1, (count + perPage - 1) / perPage);
		if (in.down & KEY_UP) page--;
		if (in.down & KEY_DOWN) page++;
		page = std::max(0, std::min(page, statsPageCount - 1));
		if (uiButton(8, 180, 64, 22, "Prev", false, page > 0)) page--;
		if (uiButton(248, 180, 64, 22, "Next", false, page + 1 < statsPageCount)) page++;
		uiTextCentered(160, 185, 0.42f, col::textDim, std::to_string(page + 1) + " / " + std::to_string(statsPageCount));
	};
	if (statsTab == 0)
	{
		std::string race;
		for (const auto& r : g.races) if (lower(r.id) == lower(s.race)) race = r.name;
		fact(58, g.gmstLabel("srace", "Race"), race + (s.female ? " (female)" : " (male)"));
		fact(80, g.gmstLabel("sclass", "Class"), cls ? cls->name : "-");
		const BirthDef* sign = w.birthsign();
		fact(102, g.gmstLabel("sbirthsign", "Birthsign"), sign ? sign->name : "-");
		int total = levelUpTotal();
		uiText(12, 126, 0.48f, col::header, g.gmstLabel("slevel", "Level") + " " + std::to_string(s.level));
		uiTextRight(306, 128, 0.4f, col::text, std::to_string(s.levelProgress) + " / " + std::to_string(total));
		uiBar(100, 133, 152, 7, total > 0 ? (float)s.levelProgress / total : 0.0f, col::header);
		const char* labels[] = { "Health", "Magicka", "Fatigue" };
		float cur[] = { s.health, s.magicka, s.fatigue }, max[] = { s.healthMax, s.magickaMax, s.fatigueMax };
		u32 colors[] = { col::health, col::magicka, col::fatigue };
		for (int i = 0; i < 3; i++)
		{
			float x = 8 + i * 104;
			uiTextCentered(x + 48, 154, 0.4f, col::text, labels[i]);
			uiBar(x, 174, 96, 14, max[i] > 0 ? cur[i] / max[i] : 0.0f, colors[i]);
			uiTextCentered(x + 48, 190, 0.36f, col::text, std::to_string((int)cur[i]) + " / " + std::to_string((int)max[i]));
		}
	}
	else if (statsTab == 1)
	{
		uiText(12, 56, 0.38f, col::textDim, "Attribute");
		uiTextRight(306, 56, 0.38f, col::textDim, "Current / Base");
		for (int i = 0; i < 8; i++)
		{
			float y = 72 + i * 16;
			if (i % 2 == 0) uiRect(8, y, 304, 16, col::panelLight);
			int base = s.attrCreation[i] + s.attrBonus[i];
			u32 color = s.attributes[i] < base ? col::health : s.attributes[i] > base ? col::link : col::text;
			uiText(12, y, 0.45f, col::text, attrName(w, i));
			uiTextRight(248, y, 0.45f, color, std::to_string(s.attributes[i]));
			uiText(254, y, 0.42f, col::textDim, "/");
			uiTextRight(306, y, 0.45f, col::textDim, std::to_string(base));
			statsRowsShown++;
		}
	}
	else if (statsTab == 2)
	{
		if (in.down & KEY_LEFT) { statsSkillGroup = (statsSkillGroup + 2) % 3; statsSkillPage = 0; }
		if (in.down & KEY_RIGHT) { statsSkillGroup = (statsSkillGroup + 1) % 3; statsSkillPage = 0; }
		const char* groups[] = { "Major", "Minor", "Misc" };
		for (int i = 0; i < 3; i++)
			if (uiButton(8 + i * 104, 55, 96, 20, groups[i], statsSkillGroup == i) && statsSkillGroup != i)
			{ statsSkillGroup = i; statsSkillPage = 0; }
		std::vector<int> skills;
		auto contains = [](const int* a, int k) { return std::find(a, a + 5, k) != a + 5; };
		for (int k = 0; k < 27; k++)
		{
			bool major = cls && contains(cls->major, k), minor = cls && contains(cls->minor, k);
			if ((statsSkillGroup == 0 && major) || (statsSkillGroup == 1 && minor)
				|| (statsSkillGroup == 2 && !major && !minor)) skills.push_back(k);
		}
		pager(statsSkillPage, skills.size(), 5);
		for (int row = 0, n = statsSkillPage * 5; n < (int)skills.size() && row < 5; n++, row++)
		{
			int k = skills[n];
			float y = 78 + row * 20;
			if (row % 2 == 0) uiRect(8, y, 304, 20, col::panelLight);
			uiText(12, y, 0.45f, col::text, statsFit(skillName(w, k), 185, 0.45f));
			uiTextRight(306, y, 0.45f, col::header, std::to_string(s.skills[k]));
			float need = skillNeed(k);
			uiBar(204, y + 14, 102, 4, s.skills[k] >= 100 ? 1.0f : need > 0 ? s.skillProgress[k] / need : 0.0f, col::header);
			statsRowsShown++;
		}
		if (skills.empty()) uiTextCentered(160, 112, 0.45f, col::textDim, "No skills in this group.");
	}
	else
	{
		uiText(12, 56, 0.4f, col::text, "Reputation: " + std::to_string(w.pcReputation));
		uiTextRight(306, 56, 0.4f, w.bounty ? col::health : col::text, "Bounty: " + std::to_string(w.bounty));
		std::vector<std::string> factions;
		for (const auto& f : w.pcRank) factions.push_back(f.first);
		std::sort(factions.begin(), factions.end(), [&](const std::string& a, const std::string& b) {
			return lower(g.factionName(a)) < lower(g.factionName(b));
		});
		pager(statsFactionPage, factions.size(), 4);
		for (int row = 0, n = statsFactionPage * 4; n < (int)factions.size() && row < 4; n++, row++)
		{
			const auto& id = factions[n];
			float y = 78 + row * 25;
			if (row % 2 == 0) uiRect(8, y, 304, 24, col::panelLight);
			bool expelled = w.pcExpelled.count(id);
			uiText(12, y, 0.44f, expelled ? col::health : col::header, statsFit(g.factionName(id), 212, 0.44f));
			auto rep = w.pcFacRep.find(id);
			uiTextRight(306, y, 0.36f, col::textDim, "Rep " + std::to_string(rep == w.pcFacRep.end() ? 0 : rep->second));
			std::string rank = g.rankName(id, w.pcRank.at(id)) + (expelled ? " - expelled" : "");
			uiText(12, y + 12, 0.38f, col::text, statsFit(rank, 292, 0.38f));
			statsRowsShown++;
		}
		if (factions.empty()) uiTextCentered(160, 110, 0.45f, col::textDim, "No faction memberships.");
	}
	if (buttonRow({ g.gmst("sclose", "Close") }, focus) == 0 || (in.down & KEY_B)) closeScreen();
}
