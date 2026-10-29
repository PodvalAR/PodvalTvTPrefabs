[EntityEditorProps(category: "GameScripted/PodvalTvT", description: "Documents excluded from building interior destruction")]
class PTVT_ProtectedDocumentEntityClass : GenericEntityClass
{
}

class PTVT_ProtectedDocumentEntity : GenericEntity
{
}

modded class SCR_BuildingDestructionManagerComponent
{
	// Both building collapse and late-join interior cleanup use this exclusion list.
	override array<typename> GetExcludedQueryTypes()
	{
		array<typename> excludedTypes = super.GetExcludedQueryTypes();
		if (!excludedTypes.Contains(PTVT_ProtectedDocumentEntity))
			excludedTypes.Insert(PTVT_ProtectedDocumentEntity);

		return excludedTypes;
	}
}
