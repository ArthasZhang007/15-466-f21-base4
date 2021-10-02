#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "View.hpp"

#include <glm/glm.hpp>

#include <map>
#include <vector>
#include <deque>


// Reference: 
struct Story {
    struct Line {
		Line(int idx_, std::string line_) : character_idx(idx_), line(line_) {};
		size_t character_idx;
		std::string line;
	};

	struct Branch {
		std::vector<Line> lines;
		size_t line_idx = 0;        // current line index

		// options, ending will have zero length options
		std::vector<std::string> option_lines;
		std::vector<std::string> next_branch_names;
	};

    // all branches stored in the story
    std::map<std::string, Branch> stories;

    

}