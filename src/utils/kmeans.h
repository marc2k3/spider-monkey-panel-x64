#pragma once

namespace smp::utils::kmeans
{
	struct PointData
	{
		PointData() = default;
		PointData(const std::vector<uint8_t>& values, size_t pixel_count);

		std::vector<uint8_t> values;
		size_t pixel_count{};
	};

	struct ClusterData
	{
		std::vector<uint8_t> central_values;
		std::vector<const PointData*> points;
	};

	std::vector<ClusterData> run(const std::vector<PointData>& points, size_t K, size_t max_iterations);
}
