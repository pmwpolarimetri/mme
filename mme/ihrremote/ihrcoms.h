#pragma once

namespace mme::detail {

	struct ChangeWavelength {
		double value;
	};

	struct Wavelength {
		double value;
	};
	
	struct ReadWavelength {};

	struct Initialize {
		bool forced = false;
	};

} //namespace mme::detail