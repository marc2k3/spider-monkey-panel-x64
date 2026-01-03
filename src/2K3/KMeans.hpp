#pragma once
#include <utils/colour_helpers.h>

static constexpr size_t kNumColourComponents = 3uz;
static constexpr size_t kMaxIterations = 12uz;
static constexpr size_t kMaxCount = 14uz;
static constexpr std::array kMultipliers = { 2.0, 4.0, 3.0 };
using ColourValues = std::array<double, kNumColourComponents>;

class KPoint
{
public:
	KPoint(size_t point_id, const ColourValues& values, size_t pixel_count)
		: m_point_id(point_id)
		, m_values(values)
		, m_pixel_count(pixel_count)
		, m_cluster_id(SIZE_MAX) {}

	ColourValues m_values;
	size_t m_cluster_id{}, m_pixel_count{}, m_point_id{};
};

using KPoints = std::vector<KPoint>;

class Cluster
{
public:
	Cluster() {}

	Cluster(const KPoint& point) : central_values(point.m_values)
	{
		points.emplace_back(point);
	}

	double calc_dist(const KPoint& point) const
	{
		double sum = 0.0;

		for (const auto i : indices(kNumColourComponents))
		{
			sum += square_then_multiply(central_values[i] - point.m_values[i], kMultipliers[i]);
		}

		return sum;
	}

	double get_frequency(size_t colours_length)
	{
		return get_total_points() / static_cast<double>(colours_length);
	}

	double square_then_multiply(double num, double by) const
	{
		return num * num * by;
	}

	int get_colour()
	{
		const auto rgb = RGB(get_colour_component(0u), get_colour_component(1u), get_colour_component(2u));
		return smp::ColorrefToArgb(rgb);
	}

	size_t get_total_points() const
	{
		return std::accumulate(points.begin(), points.end(), 0uz, [](size_t t, const KPoint& point)
			{
				return t + point.m_pixel_count;
			});
	}

	uint8_t get_colour_component(uint32_t index)
	{
		return static_cast<uint8_t>(central_values[index]);
	}

	void remove_point(size_t point_id)
	{
		std::erase_if(points, [point_id](const KPoint& point)
			{
				return point.m_point_id == point_id;
			});
	}

	ColourValues central_values{};
	KPoints points;
};

using Clusters = std::vector<Cluster>;

class KMeans
{
public:
	KMeans(const KPoints& points, size_t count) : m_points(points), m_count(count) {}

	Clusters run()
	{
		const auto count = std::min(std::max(m_count, kMaxCount), m_points.size());

		for (const auto i : indices(count))
		{
			const auto index_point = i * sizeu(m_points) / count;
			m_points[index_point].m_cluster_id = i;
			Cluster cluster(m_points[index_point]);
			m_clusters.emplace_back(cluster);
		}

		for ([[maybe_unused]] const auto i : indices(kMaxIterations))
		{
			bool done = true;

			for (KPoint& point : m_points)
			{
				const auto old_cluster_id = point.m_cluster_id;
				const auto nearest_centre_id = get_nearest_centre_id(point);

				if (old_cluster_id != nearest_centre_id)
				{
					if (old_cluster_id != SIZE_MAX)
					{
						m_clusters[old_cluster_id].remove_point(point.m_point_id);
					}

					point.m_cluster_id = nearest_centre_id;
					m_clusters[nearest_centre_id].points.emplace_back(point);
					done = false;
				}
			}

			for (Cluster& cluster : m_clusters)
			{
				for (const auto j : indices(kNumColourComponents))
				{
					const auto cluster_total_points = cluster.get_total_points();

					if (cluster_total_points == 0)
						continue;

					const double sum = std::accumulate(cluster.points.begin(), cluster.points.end(), 0.0, [j](double t, const KPoint& point)
						{
							return t + (point.m_values[j] * point.m_pixel_count);
						});

					cluster.central_values[j] = sum / cluster_total_points;
				}
			}

			if (done) break;
		}

		std::ranges::sort(m_clusters, [](const Cluster& a, const Cluster& b)
			{
				return a.get_total_points() > b.get_total_points();
			});

		if (m_count < m_clusters.size())
		{
			m_clusters.resize(m_count);
		}

		return m_clusters;
	}

private:
	size_t get_nearest_centre_id(const KPoint& point)
	{
		auto view = m_clusters | std::views::transform([point](const Cluster& cluster)
			{
				return cluster.calc_dist(point);
			});

		const auto it = std::ranges::min_element(view);
		return std::ranges::distance(view.begin(), it);
	}

	Clusters m_clusters;
	KPoints m_points;
	size_t m_count{};
};
