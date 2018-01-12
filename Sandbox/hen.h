#pragma once

#include <memory>

struct Hen
{
	unsigned m_id;
	float m_eggs;

	Hen(unsigned const id_, float eggs_) :
		m_id{ id_ },
		m_eggs{ eggs_ }
	{}

	~Hen()
	{
		TRACE(L"Chicken soup!\n");
	}
};

auto GetHen() -> std::unique_ptr<Hen>
{
	return std::make_unique<Hen>(2, 3.9f);
}

auto UpdateHen(std::unique_ptr<Hen> _hen) -> std::unique_ptr<Hen>
{
	_hen->m_eggs += 1.8f;
	return _hen;
}