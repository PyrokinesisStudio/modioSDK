#include "c++/ModIOInstance.h"

namespace modio
{
	void Instance::getAllModDependencies(u32 mod_id, const std::function<void(const modio::Response& response, const std::vector<modio::Dependency> & mods)>& callback)
	{
		const struct GetAllModDependenciesCall* get_all_mod_dependencies_call = new GetAllModDependenciesCall{ callback };
		get_all_mod_dependencies_calls[this->current_call_id] = (GetAllModDependenciesCall*)get_all_mod_dependencies_call;

		modioGetAllModDependencies((void*)new u32(this->current_call_id), mod_id, &onGetAllModDependencies);
		
		this->current_call_id++;
	}

	void Instance::addModDependencies(u32 mod_id, std::vector<u32> dependencies, const std::function<void(const modio::Response& response)>& callback)
	{
		u32* dependencies_array = new u32[dependencies.size()];
		for (int i = 0; i<(int)dependencies.size(); i++)
		{
			dependencies_array[i] = dependencies[i];
		}

		const struct AddModDependenciesCall* add_mod_dependencies_call = new AddModDependenciesCall{ callback };
		add_mod_dependencies_calls[this->current_call_id] = (AddModDependenciesCall*)add_mod_dependencies_call;

		modioAddModDependencies((void*)new u32(this->current_call_id), mod_id, dependencies_array, (u32)dependencies.size(), &onAddModDependencies);		
		this->current_call_id++;

		delete[] dependencies_array;
	}

	void Instance::deleteModDependencies(u32 mod_id, std::vector<u32> dependencies, const std::function<void(const modio::Response& response)>& callback)
	{
		u32* dependencies_array = new u32[dependencies.size()];

		for (int i = 0; i<(int)dependencies.size(); i++)
		{
			dependencies_array[i] = dependencies[i];
		}

		const struct DeleteModDependenciesCall* delete_mod_dependencies_call = new DeleteModDependenciesCall{ callback };
		delete_mod_dependencies_calls[this->current_call_id] = (DeleteModDependenciesCall*)delete_mod_dependencies_call;

		modioDeleteModDependencies((void*)new u32(this->current_call_id), mod_id, dependencies_array, (u32)dependencies.size(), &onDeleteModDependencies);
		this->current_call_id++;

		delete[] dependencies_array;
	}
}
