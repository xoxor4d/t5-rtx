#pragma once

namespace components::sp
{
	class ceg : public component
	{
	public:
		ceg();
		~ceg() = default;
		const char* get_name() override { return "ceg_sp"; }
	};
}
