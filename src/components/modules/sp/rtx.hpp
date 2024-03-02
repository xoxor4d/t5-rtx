#pragma once

namespace components::sp
{
	class rtx : public component
	{
	public:
		rtx();
		~rtx() = default;
		const char* get_name() override { return "rtx_sp"; }
	};
}
