#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <unordered_set>
#include <chrono>
#include <ctime>

#include <glm/glm.hpp>
#include "flatten.h"

namespace TFDEMO {

struct trias_hash {
    std::size_t operator() (const Face& tri) const {
        auto h1 = std::hash<uint32_t>{}(tri.indices.x);
        auto h2 = std::hash<uint32_t>{}(tri.indices.y);
        auto h3 = std::hash<uint32_t>{}(tri.indices.z);
        return h1 ^ h2 ^ h3;
    }
};


void Flatten::mark_edges(
    const Vertices& vertices, const Faces& faces,
    Edges& edges_bound, Edges& edge_inner
) {
    edges_bound.clear();
    edge_inner.clear();
    std::set<OrderedEdge> edges_bound_set;
    std::set<OrderedEdge> edge_inner_set;
    // 构造Edge集合，判断只与一个三角形相邻的边
    std::unordered_map<OrderedEdge, int, pair_hash> edge_count_map;
    for (auto& tri : faces) {
        for (int i = 0; i < 3; ++i) {
            int vidx = std::min(tri.indices[i], tri.indices[(i + 1) % 3]);
            int vidx_next = std::max(tri.indices[i], tri.indices[(i + 1) % 3]);
            OrderedEdge edge(vidx, vidx_next);
            edge_count_map[edge]++;
        }
    }
    for (auto& [edge, count] : edge_count_map) {
        if (count == 1) {
            edges_bound_set.insert(edge);
        } else if (count == 2) {
            edge_inner_set.insert(edge);
        } else {
            std::cout << "Error: edge count is not 1 or 2" << std::endl;
        }
    }
    edges_bound.assign(edges_bound_set.begin(), edges_bound_set.end());
    edge_inner.assign(edge_inner_set.begin(), edge_inner_set.end());
}

void Flatten::reorder_boundary(
    const Vertices& vertices, const Faces& faces, Edges& edges_bound, float& bound_length
) {
    if (edges_bound.empty())
        return;
    // 对边缘边进行 拓扑关系排序
    // 先构造一个邻接表
    std::unordered_map<int, std::vector<int>> adj_list;
    for (auto& edge : edges_bound) {
        adj_list[edge.first].push_back(edge.second);
        adj_list[edge.second].push_back(edge.first);
    }

    // 理论上， 所有边缘点构成一个环，所有adj个数都为2
    Edges edges_bound_reorder;
    // 从任一点触发，构造一个拓扑序列
    int vidx = edges_bound[0].first;
    int vidx_reserved = vidx;
    std::set<int> visited;

    // 计算边缘总长度
    bound_length = 0;

    while (visited.count(vidx) == 0) {
        visited.insert(vidx);
        int vidx_next = adj_list[vidx][0];
        if (visited.count(adj_list[vidx][0]) != 0) {
            // 判断是否首尾相接
            if (visited.count(adj_list[vidx][1]) != 0) {
                vidx_next = vidx_reserved;
            } else {
                vidx_next = adj_list[vidx][1];
            }
        }
        edges_bound_reorder.emplace_back(OrderedEdge(vidx, vidx_next));
        bound_length += glm::length(
            vertices[vidx].position - 
            vertices[vidx_next].position
        );
        vidx = vidx_next;
    }
    std::cout << "topology reorder: " << edges_bound.size() << " == " << edges_bound_reorder.size() << std::endl;
    edges_bound.swap(edges_bound_reorder);
}

void Flatten::map_boundary(
    const Vertices& vertices, const Faces& faces, Edges& edges_bound,
    std::vector<glm::vec3>& boundary, float boundary_length, float scale
) {
    if (edges_bound.empty())
        return;
    // 参数平面 边缘点 根据edges_bound 顺序计算得到
    // 三维空间中网格的边缘会被映射到二维参数平面的单位圆/或正方形边缘
    // param_x^j = sin(\theta^j)
    // param_y^j = cos(\theta^j)
    // \theta^j = 2 * \pi (\sum_{i=1}^{j} (vb_{i + 1} - vb_{i})) /
    // m_bound_length 其中 vb_{i + 1} - vb_{i} 为边缘点的距离
    boundary.clear();
    float _accumulate_length = 0.0;
    bool disturbed_1 = false;
    bool disturbed_2 = false;
    bool disturbed_3 = false;
    for (auto& edge : edges_bound) {
        _accumulate_length +=
            glm::length(vertices[edge.first].position - vertices[edge.second].position);
    
        /**
         * mapping to rectangle bound
         * make sure every corner of rect is mapped to a vertex
         */
        glm::vec3 bound_point(0.0, 0.0, 0.0);
        float ratio = _accumulate_length / boundary_length;
        if (ratio < 0.25) {
            bound_point.z = -(scale / 2) + scale * (ratio / 0.25);
            bound_point.y = -(scale / 2);
        } else if (ratio < 0.5) {
            if (!disturbed_1) {
                disturbed_1 = true;
                ratio = 0.25;
            }
            bound_point.z = (scale / 2);
            bound_point.y = -(scale / 2) + scale * ((ratio - 0.25) / 0.25);
        } else if (ratio < 0.75) {
            if (!disturbed_2) {
                disturbed_2 = true;
                ratio = 0.5;
            }
            bound_point.z = (scale / 2) - scale * ((ratio - 0.5) / 0.25);
            bound_point.y = (scale / 2);
        } else {
            if (!disturbed_3) {
                disturbed_3 = true;
                ratio = 0.75;
            }
            bound_point.z = -(scale / 2);
            bound_point.y = (scale / 2) - scale * ((ratio - 0.75) / 0.25);
        }
        boundary.push_back(bound_point);
    }
}

void Flatten::init_weights(
    const Vertices& vertices, const Faces& faces,
    const Edges& edges_bound, const Edges& edges_inner,
    Weights& weights
) {
    std::unordered_set<Face, trias_hash> trias_set(faces.begin(), faces.end());
    /** 存在这样一个事实：
     *  vi的邻接点中有vk, vk的邻接点中有vj，可能并不存在三角形(vi, vj, vk)
     * (vi) --------------(vk)
     *  | \           /  /
     *  |  \        /  /
     *  |    -(vl)-  /
     *  |     /    /
     *  |   /    /
     *  | /   /
     * (vj)-
     */
    // 构造邻接表 快速索引
    std::unordered_map<int, std::set<int>> adj_list;
    for (auto& edge : edges_inner) {
        adj_list[edge.first].insert(edge.second);
        adj_list[edge.second].insert(edge.first);
    }
    for (auto& edge : edges_bound) {
        adj_list[edge.first].insert(edge.second);
        adj_list[edge.second].insert(edge.first);
    }

     // weights只需从inner构造
    for (auto edge : edges_inner) {
        int vi = std::min(edge.first, edge.second);
        int vj = std::max(edge.first, edge.second);
        std::vector<int> adj_vt;
        for (auto& adj_v : adj_list[vi]) {
            // 判断(adj_v, vj, vi)是否构成三角形
            if (adj_v != vj && trias_set.count(Face{glm::uvec3(vi, vj, adj_v)}) != 0) {
                adj_vt.emplace_back(adj_v);
            }
        }
        if (adj_vt.size() != 2) {
            std::cerr << "Error: edge_inner " << vi << " " << vj << ": "
                 << adj_vt.size() << std::endl;
            continue;
        }
        // compute \lambda_{ij} = D_{ij} / \sum_{k \in N_i} D_{ik}
        // assume D_{ij} = 1
        float sum_weight = 0.0f;
        for (auto& vk : adj_list[vi]) {
            sum_weight += 1.0f; // modify if D_{ij} != 1
        }
        float _weight = 1.0f / sum_weight;
        weights[OrderedEdge(vi, vj)] = -_weight;
        weights[OrderedEdge(vi, vi)] += _weight;
        weights[OrderedEdge(vj, vj)] += _weight;
    }
}

void Flatten::transfer_edges_to_vertex_indices(
    Edges&& edges_bound, Edges&& edges_inner,
    std::vector<int>& vts_bound, std::vector<int>& vts_inner
) {
    if (edges_inner.empty()) {
        return;
    }
    vts_bound.clear();
    vts_inner.clear();
    // 计算vts_bound
    // 由于边缘边拓扑有序，所以尽量避免在vts_bound中查重操作
    const auto sz = edges_bound.size();
    // 最后一个边包含的两个点都应已经被包含在vts_bound中
	if (sz != 0) {
		vts_bound.emplace_back(edges_bound[0].first);
		for (size_t eidx = 0; eidx < sz - 1; ++eidx) {
			const int _last_vt = vts_bound.back();
			int picked = -1;
			if (edges_bound[eidx].first == _last_vt) {
				picked = edges_bound[eidx].second;
			}
			else if (edges_bound[eidx].second == _last_vt) {
				picked = edges_bound[eidx].first;
			}
			if (picked == -1) {
				std::cout << "Error: edges_bound is not topology sorted" << std::endl;
				continue;
			}
			if (find(vts_bound.begin(), vts_bound.end(), picked) == vts_bound.end()) {
				vts_bound.emplace_back(picked);
			}
		}
	}

    // 计算vt_inner
    std::unordered_set<int> vts_bound_set(vts_bound.begin(), vts_bound.end());
    std::unordered_set<int> vts_inner_set;
    for (auto& edge : edges_inner) {
        if (vts_bound_set.count(edge.first) == 0) {
            vts_inner_set.insert(edge.first);
        }
        if (vts_bound_set.count(edge.second) == 0) {
            vts_inner_set.insert(edge.second);
        }
    }

    vts_inner.assign(vts_inner_set.begin(), vts_inner_set.end());
    // 释放内存
    Edges().swap(edges_bound);
    Edges().swap(edges_inner);
}


}