#include <stdafx.h>
#include "kmeans.h"

namespace
{
	using namespace smp::utils::kmeans;

	struct Point
	{
		explicit Point(const PointData* pData) : pData(pData) {}

		const PointData* pData;
		size_t id_cluster = SIZE_MAX;
	};

	struct Cluster
	{
		Cluster(size_t id_cluster, const Point* pPoint) : id_cluster(id_cluster)
		{
			central_values =
				ranges::views::transform(pPoint->pData->values, [](const auto& elem) { return static_cast<double>(elem); })
				| ranges::to_vector;
			points.push_back(pPoint);
		}

		size_t id_cluster;
		std::vector<double> central_values;
		std::vector<const Point*> points;
	};

	// return ID of nearest center
	// uses distance calculations from: https://en.wikipedia.org/wiki/Color_difference
	size_t getIDNearestCenter(const std::vector<Cluster>& clusters, const Point& point)
	{
		const auto& pointValues = point.pData->values;

		auto calculateDistance = [&pointValues](const Cluster& cluster) {
			double sum = 0.0;
			const auto& centralValues = cluster.central_values;

			sum += 2 * pow(centralValues[0] - pointValues[0], 2.0); // r
			sum += 4 * pow(centralValues[1] - pointValues[1], 2.0); // g
			sum += 3 * pow(centralValues[2] - pointValues[2], 2.0); // b

			return sum;
		};

		size_t id_cluster_center{};
		double min_dist = calculateDistance(clusters[0]);

		for (auto i: ranges::views::indices(clusters.size()))
		{
			double dist = calculateDistance(clusters[i]);
			if (dist < min_dist)
			{
				min_dist = dist;
				id_cluster_center = i;
			}
		}

		return id_cluster_center;
	}

	size_t getTotalPixelCount(const Cluster& cluster)
	{
		return ranges::accumulate(cluster.points, 0uz, [](size_t sum, const auto pPoint)
			{
				return sum + pPoint->pData->pixel_count;
			});
	}
}

namespace smp::utils::kmeans
{
	PointData::PointData(const std::vector<uint8_t>& values, size_t pixel_count) : values(values), pixel_count(pixel_count) {}

	std::vector<ClusterData> run(const std::vector<PointData>& pointsData, size_t K, size_t max_iterations)
	{
		const size_t clusterCount = std::min(std::max(K, 14uz), pointsData.size());

		auto points =
			ranges::views::transform(pointsData, [](const auto& data) { return Point{ &data }; })
			| ranges::to_vector;
		std::vector<Cluster> clusters;
		clusters.reserve(clusterCount);

		// choose K distinct values for the centers of the clusters
		for (auto i: ranges::views::indices(clusterCount))
		{ // colours are already distinct so we can't have duplicate centers
			auto& centerPoint = points[static_cast<size_t>(i * points.size() / clusterCount)];
			centerPoint.id_cluster = i;
			clusters.emplace_back(i, &centerPoint);
		}

		for (auto i: ranges::views::indices(max_iterations))
		{
			(void)i;
			bool done = true;

			// associate each point to its nearest center
			for (auto& point: points)
			{
				const auto id_old_cluster = point.id_cluster;
				const auto id_nearest_center = getIDNearestCenter(clusters, point);

				if (id_old_cluster != id_nearest_center)
				{
					if (id_old_cluster != SIZE_MAX)
					{
						auto& clusterPoints = clusters[id_old_cluster].points;
						const auto it = ranges::find(clusterPoints, &point);
						assert(it != clusterPoints.cend());
						clusterPoints.erase(it);
					}

					point.id_cluster = id_nearest_center;
					clusters[id_nearest_center].points.push_back(&point);
					done = false;
				}
			}

			// recalculating the center of each cluster
			for (auto& cluster : clusters)
			{
				const auto pixelsInCluster = getTotalPixelCount(cluster);
				if (!pixelsInCluster)
				{
					continue;
				}

				for (auto&& [j, centralValue]: ranges::views::enumerate(cluster.central_values))
				{
					const auto sum = ranges::accumulate(cluster.points, 0uz, [j = j](size_t curSum, const auto pPoint) {
						return curSum + pPoint->pData->values[j] * pPoint->pData->pixel_count;
					});
					centralValue = static_cast<double>(sum) / pixelsInCluster;
				}
			}

			if (done)
			{
				break;
			}
		}

		return ranges::views::transform(clusters, [](const auto& cluster) {
				ClusterData clusterData;
				clusterData.central_values = ranges::views::transform(cluster.central_values, [](const auto& value) { return static_cast<uint8_t>(value); }) | ranges::to_vector;
				clusterData.points = ranges::views::transform(cluster.points, [](const auto& point) { return point->pData; }) | ranges::to_vector;
				return clusterData;
			}) | ranges::to_vector;
	}
}
