#pragma once

#include "resource.h"

#include <iostream>
#include <linalg.h>
#include <memory>
#include <omp.h>
#include <random>

using namespace linalg::aliases;

namespace cg::renderer
{
	struct ray
	{
		ray(float3 position, float3 direction) : position(position)
		{
			this->direction = normalize(direction);
		}
		float3 position;
		float3 direction;
	};

	struct payload
	{
		float t;
		float3 bary;
		cg::color color;
	};

	template<typename VB>
	struct triangle
	{
		triangle(const VB& vertex_a, const VB& vertex_b, const VB& vertex_c);

		float3 a;
		float3 b;
		float3 c;

		float3 ba;
		float3 ca;

		float3 na;
		float3 nb;
		float3 nc;

		float3 ambient;
		float3 diffuse;
		float3 emissive;
	};

	template<typename VB>
	inline triangle<VB>::triangle(
			const VB& vertex_a, const VB& vertex_b, const VB& vertex_c)
	{
		// TODO: Lab 2.02. Implement a constructor of triangle struct
	}

	template<typename VB>
	class aabb
	{
	public:
		void add_triangle(const triangle<VB> triangle);
		const std::vector<triangle<VB>>& get_triangles() const;
		bool aabb_test(const ray& ray) const;

	protected:
		std::vector<triangle<VB>> triangles;

		float3 aabb_min;
		float3 aabb_max;
	};

	struct light
	{
		float3 position;
		float3 color;
	};

	template<typename VB, typename RT>
	class raytracer
	{
	public:
		raytracer(){};
		~raytracer(){};

		void set_render_target(std::shared_ptr<resource<RT>> in_render_target);
		void clear_render_target(const RT& in_clear_value);
		void set_viewport(size_t in_width, size_t in_height);

		void set_vertex_buffers(std::vector<std::shared_ptr<cg::resource<VB>>> in_vertex_buffers);
		void set_index_buffers(std::vector<std::shared_ptr<cg::resource<unsigned int>>> in_index_buffers);
		void build_acceleration_structure();
		std::vector<aabb<VB>> acceleration_structures;

		void ray_generation(float3 position, float3 direction, float3 right, float3 up, size_t depth, size_t accumulation_num);

		payload trace_ray(const ray& ray, size_t depth, float max_t = 1000.f, float min_t = 0.001f) const;
		payload intersection_shader(const triangle<VB>& triangle, const ray& ray) const;

		std::function<payload(const ray& ray)> miss_shader = nullptr;
		std::function<payload(const ray& ray, payload& payload, const triangle<VB>& triangle, size_t depth)>
				closest_hit_shader = nullptr;
		std::function<payload(const ray& ray, payload& payload, const triangle<VB>& triangle)> any_hit_shader =
				nullptr;

		float2 get_jitter(int frame_id);

	protected:
		std::shared_ptr<cg::resource<RT>> render_target;
		std::shared_ptr<cg::resource<float3>> history;
		std::vector<std::shared_ptr<cg::resource<unsigned int>>> index_buffers;
		std::vector<std::shared_ptr<cg::resource<VB>>> vertex_buffers;

		size_t width = 1920;
		size_t height = 1080;
	};

	template<typename VB, typename RT>
	inline void raytracer<VB, RT>::set_render_target(
			std::shared_ptr<resource<RT>> in_render_target)
	{
		render_target = in_render_target;
	}

	template<typename VB, typename RT>
	inline void raytracer<VB, RT>::clear_render_target(
			const RT& in_clear_value)
	{
		for (size_t i = 0; i < render_target->get_number_of_elements(); i++)
		{
			render_target->item(i) = in_clear_value;
		}
		// TODO: Lab 2.06. Add `history` resource in `raytracer` class
	}
	template<typename VB, typename RT>
	void raytracer<VB, RT>::set_index_buffers(std::vector<std::shared_ptr<cg::resource<unsigned int>>> in_index_buffers)
	{
		index_buffers = in_index_buffers;
	}
	template<typename VB, typename RT>
	inline void raytracer<VB, RT>::set_vertex_buffers(std::vector<std::shared_ptr<cg::resource<VB>>> in_vertex_buffers)
	{
		vertex_buffers = in_vertex_buffers;
	}

	template<typename VB, typename RT>
	inline void raytracer<VB, RT>::build_acceleration_structure()
	{
		// TODO: Lab 2.05. Implement build_acceleration_structure method of raytracer class
	}

	template<typename VB, typename RT>
	inline void raytracer<VB, RT>::set_viewport(size_t in_width,
												size_t in_height)
	{
		height = in_height;
		width = in_width;
		// TODO: Lab 2.06. Add `history` resource in `raytracer` class
	}

	template<typename VB, typename RT>
	inline void raytracer<VB, RT>::ray_generation(
			float3 position, float3 direction,
			float3 right, float3 up, size_t depth, size_t accumulation_num)
	{
		for (int x = 0; x < width; x++)
		{
			for(int y = 0; y < height; y++)
			{
				float u = (2.f * x) / static_cast<float>(width - 1) - 1.f;
				float v = (2.f * y) / static_cast<float>(height - 1) - 1.f;
				u *= static_cast<float>(width) / static_cast<float>(height);
				float3 ray_direction = direction + u * right - v * up;
				ray ray(position, ray_direction);

				payload payload = trace_ray(ray, depth);

				render_target->item(x, y) = RT::from_color(payload.color);

			}
		}
		// TODO: Lab 2.06. Add `history` resource in `raytracer` class
		// TODO: Lab 2.06. Implement TAA in `ray_generation` method of `raytracer` class
	}

	template<typename VB, typename RT>
	inline payload raytracer<VB, RT>::trace_ray(
			const ray& ray, size_t depth, float max_t, float min_t) const
	{
		if (depth == 0)
			return miss_shader(ray);
		depth--;

		return miss_shader(ray);

		// TODO: Lab 2.02. Adjust trace_ray method of raytracer class to traverse geometry and call a closest hit shader
		// TODO: Lab 2.04. Adjust `trace_ray` method of `raytracer` to use `any_hit_shader`
		// TODO: Lab 2.05. Adjust trace_ray method of raytracer class to traverse the acceleration structure
	}

	template<typename VB, typename RT>
	inline payload raytracer<VB, RT>::intersection_shader(
			const triangle<VB>& triangle, const ray& ray) const
	{
		// TODO: Lab 2.02. Implement an intersection_shader method of raytracer class
	}

	template<typename VB, typename RT>
	float2 raytracer<VB, RT>::get_jitter(int frame_id)
	{
		// TODO: Lab 2.06. Implement `get_jitter` method of `raytracer` class
	}


	template<typename VB>
	inline void aabb<VB>::add_triangle(const triangle<VB> triangle)
	{
		// TODO: Lab 2.05. Implement aabb class
	}

	template<typename VB>
	inline const std::vector<triangle<VB>>& aabb<VB>::get_triangles() const
	{
		// TODO: Lab 2.05. Implement aabb class
	}

	template<typename VB>
	inline bool aabb<VB>::aabb_test(const ray& ray) const
	{
		// TODO: Lab 2.05. Implement aabb class
	}

}// namespace cg::renderer