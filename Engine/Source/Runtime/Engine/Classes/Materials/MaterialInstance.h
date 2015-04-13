// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceBasePropertyOverrides.h"
#include "StaticParameterSet.h"
#include "MaterialInstance.generated.h"

//
// Forward declarations.
//
class FMaterialShaderMap;
class FMaterialShaderMapId;
class FSHAHash;

/** Editable font parameter. */
USTRUCT()
struct FFontParameterValue
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=FontParameterValue)
	FName ParameterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=FontParameterValue)
	class UFont* FontValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=FontParameterValue)
	int32 FontPage;

	UPROPERTY()
	FGuid ExpressionGUID;


	FFontParameterValue()
		: FontValue(NULL)
		, FontPage(0)
	{
	}


	typedef const UTexture* ValueType;
	static ValueType GetValue(const FFontParameterValue& Parameter);
};

/** Editable scalar parameter. */
USTRUCT()
struct FScalarParameterValue
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ScalarParameterValue)
	FName ParameterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ScalarParameterValue)
	float ParameterValue;

	UPROPERTY()
	FGuid ExpressionGUID;


	FScalarParameterValue()
		: ParameterValue(0)
	{
	}


	typedef float ValueType;
	static ValueType GetValue(const FScalarParameterValue& Parameter) { return Parameter.ParameterValue; }
};

/** Editable texture parameter. */
USTRUCT()
struct FTextureParameterValue
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=TextureParameterValue)
	FName ParameterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=TextureParameterValue)
	class UTexture* ParameterValue;

	UPROPERTY()
	FGuid ExpressionGUID;


	FTextureParameterValue()
		: ParameterValue(NULL)
	{
	}


	typedef const UTexture* ValueType;
	static ValueType GetValue(const FTextureParameterValue& Parameter) { return Parameter.ParameterValue; }
};

/** Editable vector parameter. */
USTRUCT()
struct FVectorParameterValue
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=VectorParameterValue)
	FName ParameterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=VectorParameterValue)
	FLinearColor ParameterValue;

	UPROPERTY()
	FGuid ExpressionGUID;


	FVectorParameterValue()
		: ParameterValue(ForceInit)
	{
	}


	typedef FLinearColor ValueType;
	static ValueType GetValue(const FVectorParameterValue& Parameter) { return Parameter.ParameterValue; }
};

UCLASS(abstract, BlueprintType,MinimalAPI)
class UMaterialInstance : public UMaterialInterface
{
	GENERATED_UCLASS_BODY()

	/** Physical material to use for this graphics material. Used for sounds, effects etc.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MaterialInstance)
	class UPhysicalMaterial* PhysMaterial;

	/** Parent material. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=MaterialInstance, AssetRegistrySearchable)
	class UMaterialInterface* Parent;

	/**
	 * Delegate for custom static parameters getter.
	 *
	 * @param OutStaticParameterSet Parameter set to append.
	 * @param Material Material instance to collect parameters.
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FCustomStaticParametersGetterDelegate, FStaticParameterSet&, UMaterialInstance*);

	/**
	 * Delegate for custom static parameters updater.
	 *
	 * @param StaticParameterSet Parameter set to update.
	 * @param Material Material to update.
	 *
	 * @returns True if any parameter been updated. False otherwise.
	 */
	DECLARE_DELEGATE_RetVal_TwoParams(bool, FCustomParameterSetUpdaterDelegate, FStaticParameterSet&, UMaterial*);

	// Custom static parameters getter delegate.
	ENGINE_API static FCustomStaticParametersGetterDelegate CustomStaticParametersGetters;

	// An array of custom parameter set updaters.
	ENGINE_API static TArray<FCustomParameterSetUpdaterDelegate> CustomParameterSetUpdaters;

	/**
	 * Gets static parameter set for this material.
	 *
	 * @returns Static parameter set.
	 */
	ENGINE_API const FStaticParameterSet& GetStaticParameters() const;

	/**
	 * Indicates whether the instance has static permutation resources (which are required when static parameters are present) 
	 * Read directly from the rendering thread, can only be modified with the use of a FMaterialUpdateContext.
	 * When true, StaticPermutationMaterialResources will always be valid and non-null.
	 */
	UPROPERTY()
	uint32 bHasStaticPermutationResource:1;

	/** Flag to detect cycles in the material instance graph. */
	uint32 ReentrantFlag:1;

	/** Defines if SubsurfaceProfile from this instance is used or it uses the parent one. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = MaterialInstance)
	uint32 bOverrideSubsurfaceProfile:1;

	/** Unique ID for this material, used for caching during distributed lighting */
	UPROPERTY()
	FGuid ParentLightingGuid;

	/** Font parameters. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=MaterialInstance)
	TArray<struct FFontParameterValue> FontParameterValues;

	/** Scalar parameters. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=MaterialInstance)
	TArray<struct FScalarParameterValue> ScalarParameterValues;

	/** Texture parameters. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=MaterialInstance)
	TArray<struct FTextureParameterValue> TextureParameterValues;

	/** Vector parameters. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=MaterialInstance)
	TArray<struct FVectorParameterValue> VectorParameterValues;

	UPROPERTY()
	bool bOverrideBaseProperties_DEPRECATED;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=MaterialInstance)
	struct FMaterialInstanceBasePropertyOverrides BasePropertyOverrides;

	/** 
	 * FMaterialRenderProxy derivatives that represent this material instance to the renderer, when the renderer needs to fetch parameter values. 
	 * Second instance is used when selected, third when hovered.
	 */
	class FMaterialInstanceResource* Resources[3];

private:

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TArray<FGuid> ReferencedTextureGuids;
#endif // WITH_EDITORONLY_DATA

	/** Static parameter values that are overridden in this instance. */
	FStaticParameterSet StaticParameters;

	/** 
	 * Material resources used for rendering this material instance, in the case of static parameters being present.
	 * These will always be valid and non-null when bHasStaticPermutationResource is true,
	 * But only the entries affected by CacheResourceShadersForRendering will be valid for rendering.
	 * There need to be as many entries in this array as can be used simultaneously for rendering.  
	 * For example the material instance needs to support being rendered at different quality levels and feature levels within the same process.
	 */
	FMaterialResource* StaticPermutationMaterialResources[EMaterialQualityLevel::Num][ERHIFeatureLevel::Num];
#if WITH_EDITOR
	/** Material resources being cached for cooking. */
	TMap<const class ITargetPlatform*, TArray<FMaterialResource*>> CachedMaterialResourcesForCooking;
#endif
	/** Fence used to guarantee that the RT is finished using various resources in this UMaterial before cleanup. */
	FRenderCommandFence ReleaseFence;

public:
	// Begin interface IBlendableInterface
	virtual ENGINE_API void OverrideBlendableSettings(class FSceneView& View, float Weight) const override;
	// End interface IBlendableInterface

	// Begin UMaterialInterface interface.
	virtual ENGINE_API UMaterial* GetMaterial() override;
	virtual ENGINE_API const UMaterial* GetMaterial() const override;
	virtual ENGINE_API const UMaterial* GetMaterial_Concurrent(TMicRecursionGuard& RecursionGuard) const override;
	virtual ENGINE_API FMaterialResource* GetMaterialResource(ERHIFeatureLevel::Type InFeatureLevel, EMaterialQualityLevel::Type QualityLevel = EMaterialQualityLevel::Num) override;
	virtual ENGINE_API const FMaterialResource* GetMaterialResource(ERHIFeatureLevel::Type InFeatureLevel, EMaterialQualityLevel::Type QualityLevel = EMaterialQualityLevel::Num) const override;
	virtual ENGINE_API bool GetFontParameterValue(FName ParameterName, class UFont*& OutFontValue, int32& OutFontPage) const override;
	virtual ENGINE_API bool GetScalarParameterValue(FName ParameterName, float& OutValue) const override;
	virtual ENGINE_API bool GetTextureParameterValue(FName ParameterName, class UTexture*& OutValue) const override;
	virtual ENGINE_API bool GetVectorParameterValue(FName ParameterName, FLinearColor& OutValue) const override;
	virtual ENGINE_API void GetUsedTextures(TArray<UTexture*>& OutTextures, EMaterialQualityLevel::Type QualityLevel, bool bAllQualityLevels, ERHIFeatureLevel::Type FeatureLevel, bool bAllFeatureLevels) const override;
	virtual ENGINE_API void OverrideTexture(const UTexture* InTextureToOverride, UTexture* OverrideTexture, ERHIFeatureLevel::Type InFeatureLevel) override;
	virtual ENGINE_API void OverrideVectorParameterDefault(FName ParameterName, const FLinearColor& Value, bool bOverride, ERHIFeatureLevel::Type FeatureLevel) override;
	virtual ENGINE_API void OverrideScalarParameterDefault(FName ParameterName, float Value, bool bOverride, ERHIFeatureLevel::Type FeatureLevel) override;
	virtual ENGINE_API bool CheckMaterialUsage(const EMaterialUsage Usage, const bool bSkipPrim = false) override;
	virtual ENGINE_API bool CheckMaterialUsage_Concurrent(const EMaterialUsage Usage, const bool bSkipPrim = false) const override;
	virtual ENGINE_API bool GetStaticSwitchParameterValue(FName ParameterName, bool &OutValue, FGuid &OutExpressionGuid) const override;
	virtual ENGINE_API bool GetStaticComponentMaskParameterValue(FName ParameterName, bool &R, bool &G, bool &B, bool &A, FGuid &OutExpressionGuid) const override;
	virtual ENGINE_API bool GetTerrainLayerWeightParameterValue(FName ParameterName, int32& OutWeightmapIndex, FGuid &OutExpressionGuid) const override;
	virtual ENGINE_API bool IsDependent(UMaterialInterface* TestDependency) override;
	virtual ENGINE_API FMaterialRenderProxy* GetRenderProxy(bool Selected, bool bHovered = false) const override;
	virtual ENGINE_API UPhysicalMaterial* GetPhysicalMaterial() const override;
	virtual ENGINE_API bool UpdateLightmassTextureTracking() override;
	virtual ENGINE_API bool GetCastShadowAsMasked() const override;
	virtual ENGINE_API float GetEmissiveBoost() const override;
	virtual ENGINE_API float GetDiffuseBoost() const override;
	virtual ENGINE_API float GetExportResolutionScale() const override;
	virtual ENGINE_API float GetDistanceFieldPenumbraScale() const override;
	virtual ENGINE_API bool GetTexturesInPropertyChain(EMaterialProperty InProperty, TArray<UTexture*>& OutTextures,
		TArray<FName>* OutTextureParamNames, class FStaticParameterSet* InStaticParameterSet) override;
	virtual ENGINE_API void RecacheUniformExpressions() const override;
	virtual ENGINE_API bool GetRefractionSettings(float& OutBiasValue) const override;
	ENGINE_API virtual void ForceRecompileForRendering() override;
	
	ENGINE_API virtual float GetOpacityMaskClipValue(bool bIsGameThread=IsInGameThread()) const override;
	ENGINE_API virtual EBlendMode GetBlendMode(bool bIsGameThread = IsInGameThread()) const override;
	ENGINE_API virtual EMaterialShadingModel GetShadingModel(bool bIsGameThread = IsInGameThread()) const override;
	ENGINE_API virtual bool IsTwoSided(bool bIsGameThread = IsInGameThread()) const override;
	ENGINE_API virtual bool IsMasked(bool bIsGameThread = IsInGameThread()) const override;

	ENGINE_API float RenderThread_GetOpacityMaskClipValue() const;
	ENGINE_API EBlendMode RenderThread_GetBlendMode() const;
	ENGINE_API EMaterialShadingModel RenderThread_GetShadingModel() const;
	ENGINE_API bool RenderThread_IsTwoSided() const;
	ENGINE_API bool RenderThread_IsMasked() const;
	
	ENGINE_API virtual USubsurfaceProfile* GetSubsurfaceProfile_Internal() const override;

	/** Checks to see if an input property should be active, based on the state of the material */
	ENGINE_API virtual bool IsPropertyActive(EMaterialProperty InProperty) const override;
	/** Allows material properties to be compiled with the option of being overridden by the material attributes input. */
	ENGINE_API virtual int32 CompilePropertyEx(class FMaterialCompiler* Compiler, EMaterialProperty Property) override;
	// End UMaterialInterface interface.

	// Begin UObject interface.
	virtual ENGINE_API SIZE_T GetResourceSize(EResourceSizeMode::Type Mode) override;
	virtual ENGINE_API void PostInitProperties() override;	
#if WITH_EDITOR
	virtual ENGINE_API void BeginCacheForCookedPlatformData(const ITargetPlatform *TargetPlatform) override;
	virtual ENGINE_API bool IsCachedCookedPlatformDataLoaded(const ITargetPlatform* TargetPlatform) override;
	virtual ENGINE_API void ClearCachedCookedPlatformData(const ITargetPlatform *TargetPlatform) override;
	virtual ENGINE_API void ClearAllCachedCookedPlatformData() override;
#endif
	virtual ENGINE_API void Serialize(FArchive& Ar) override;
	virtual ENGINE_API void PostLoad() override;
	virtual ENGINE_API void BeginDestroy() override;
	virtual ENGINE_API bool IsReadyForFinishDestroy() override;
	virtual ENGINE_API void FinishDestroy() override;
	ENGINE_API static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
#if WITH_EDITOR
	virtual ENGINE_API void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * Sets new static parameter overrides on the instance and recompiles the static permutation resources if needed (can be forced with bForceRecompile).
	 * Can be passed either a minimal parameter set (overridden parameters only) or the entire set generated by GetStaticParameterValues().
	 */
	ENGINE_API void UpdateStaticPermutation(const FStaticParameterSet& NewParameters, bool bForceRecompile = false);

#endif // WITH_EDITOR

	// End UObject interface.

	/**
	 * Recompiles static permutations if necessary.
	 * Note: This modifies material variables used for rendering and is assumed to be called within a FMaterialUpdateContext!
	 */
	ENGINE_API void InitStaticPermutation();

	ENGINE_API void UpdateOverridableBaseProperties();

	/** 
	 * Cache resource shaders for rendering on the given shader platform. 
	 * If a matching shader map is not found in memory or the DDC, a new one will be compiled.
	 * The results will be applied to this FMaterial in the renderer when they are finished compiling.
	 * Note: This modifies material variables used for rendering and is assumed to be called within a FMaterialUpdateContext!
	 */
	void CacheResourceShadersForCooking(EShaderPlatform ShaderPlatform, TArray<FMaterialResource*>& OutCachedMaterialResources);

	/** 
	 * Gathers actively used shader maps from all material resources used by this material instance
	 * Note - not refcounting the shader maps so the references must not be used after material resources are modified (compilation, loading, etc)
	 */
	void GetAllShaderMaps(TArray<FMaterialShaderMap*>& OutShaderMaps);

	/**
	 * Builds a composited set of static parameters, including inherited and overridden values
	 */
	ENGINE_API void GetStaticParameterValues(FStaticParameterSet& OutStaticParameters);

	void GetAllStaticSwitchParameterValues(TArray<FStaticTerrainLayerWeightParameter> &TerrainLayerWeightParameters, UMaterial* ParentMaterial);

	void GetBasePropertyOverridesHash(FSHAHash& OutHash)const;
	bool HasOverridenBaseProperties()const;

	// For all materials instances, UMaterialInstance::CacheResourceShadersForRendering
	static void AllMaterialsCacheResourceShadersForRendering();

	/** 
	 * Determine whether this Material Instance is a child of another Material
	 *
	 * @param	Material	Material to check against
	 * @return	true if this Material Instance is a child of the other Material.
	 */
	ENGINE_API bool IsChildOf(const UMaterialInterface* Material) const;

protected:
	/**
	 * Updates parameter names on the material instance, returns true if parameters have changed.
	 */
	bool UpdateParameters();

	ENGINE_API void SetParentInternal(class UMaterialInterface* NewParent);

	void GetTextureExpressionValues(const FMaterialResource* MaterialResource, TArray<UTexture*>& OutTextures) const;

	/** 
	 * Updates StaticPermutationMaterialResources based on the value of bHasStaticPermutationResource
	 * This is a helper used when recompiling MI's with static parameters.  
	 * Assumes that the rendering thread command queue has been flushed by the caller.
	 */
	void UpdatePermutationAllocations();

#if WITH_EDITOR
	/**
	* Refresh parameter names using the stored reference to the expression object for the parameter.
	*/
	ENGINE_API void UpdateParameterNames();

#endif

	/**
	 * Internal interface for setting / updating values for material instances.
	 */
	void SetVectorParameterValueInternal(FName ParameterName, FLinearColor Value);
	void SetScalarParameterValueInternal(FName ParameterName, float Value);
	void SetTextureParameterValueInternal(FName ParameterName, class UTexture* Value);
	void SetFontParameterValueInternal(FName ParameterName, class UFont* FontValue, int32 FontPage);
	void ClearParameterValuesInternal();

	/** Initialize the material instance's resources. */
	ENGINE_API void InitResources();

	/** 
	 * Cache resource shaders for rendering on the given shader platform. 
	 * If a matching shader map is not found in memory or the DDC, a new one will be compiled.
	 * The results will be applied to this FMaterial in the renderer when they are finished compiling.
	 * Note: This modifies material variables used for rendering and is assumed to be called within a FMaterialUpdateContext!
	 */
	void CacheResourceShadersForRendering();

	/** Caches shader maps for an array of material resources. */
	void CacheShadersForResources(EShaderPlatform ShaderPlatform, const TArray<FMaterialResource*>& ResourcesToCache, bool bApplyCompletedShaderMapForRendering);

	/** Copies over parameters given a material interface */
	ENGINE_API void CopyMaterialInstanceParameters(UMaterialInterface* MaterialInterface);

	// to share code between PostLoad() and PostEditChangeProperty()
	void PropagateDataToMaterialProxy();

	void SetTonemapperPostprocessMaterialSettings(class FSceneView& View, UMaterialInstanceDynamic& MID) const;

	/** Allow resource to access private members. */
	friend class FMaterialInstanceResource;
	/** Editor-only access to private members. */
	friend class FMaterialEditor;
	/** Class that knows how to update MI's */
	friend class FMaterialUpdateContext;
};

/**
 * This function takes a array of parameter structs and attempts to establish a reference to the expression object each parameter represents.
 * If a reference exists, the function checks to see if the parameter has been renamed.
 *
 * @param Parameters		Array of parameters to operate on.
 * @param ParentMaterial	Parent material to search in for expressions.
 *
 * @return Returns whether or not any of the parameters was changed.
 */
template <typename ParameterType, typename ExpressionType>
bool UpdateParameterSet(TArray<ParameterType> &Parameters, UMaterial* ParentMaterial)
{
	bool bChanged = false;

	// Loop through all of the parameters and try to either establish a reference to the 
	// expression the parameter represents, or check to see if the parameter's name has changed.
	for (int32 ParameterIdx = 0; ParameterIdx < Parameters.Num(); ParameterIdx++)
	{
		bool bTryToFindByName = true;

		ParameterType &Parameter = Parameters[ParameterIdx];

		if (Parameter.ExpressionGUID.IsValid())
		{
			ExpressionType* Expression = ParentMaterial->FindExpressionByGUID<ExpressionType>(Parameter.ExpressionGUID);

			// Check to see if the parameter name was changed.
			if (Expression)
			{
				bTryToFindByName = false;

				if (Parameter.ParameterName != Expression->ParameterName)
				{
					Parameter.ParameterName = Expression->ParameterName;
					bChanged = true;
				}
			}
		}

		// No reference to the material expression exists, so try to find one in the material expression's array if we are in the editor.
		if (bTryToFindByName && GIsEditor && !FApp::IsGame())
		{
			for (const UMaterialExpression* Expression : ParentMaterial->Expressions)
			{
				if (Expression->IsA<ExpressionType>())
				{
					const ExpressionType* ParameterExpression = CastChecked<const ExpressionType>(Expression);
					if (ParameterExpression->ParameterName == Parameter.ParameterName)
					{
						Parameter.ExpressionGUID = ParameterExpression->ExpressionGUID;
						bChanged = true;
						break;
					}
				}
				else if (const UMaterialExpressionMaterialFunctionCall* FunctionCall = Cast<const UMaterialExpressionMaterialFunctionCall>(Expression))
				{
					if (FunctionCall->MaterialFunction)
					{
						auto UpdateParameterSetInFunctions = [&]()
						{
							TArray<UMaterialFunction*> Functions;
							Functions.Add(FunctionCall->MaterialFunction);
							FunctionCall->MaterialFunction->GetDependentFunctions(Functions);

							for (UMaterialFunction* Function : Functions)
							{
								for (const UMaterialExpression* FunctionExpression : Function->FunctionExpressions)
								{
									if (const ExpressionType* ParameterExpression = Cast<const ExpressionType>(FunctionExpression))
									{
										if (ParameterExpression->ParameterName == Parameter.ParameterName)
										{
											Parameter.ExpressionGUID = ParameterExpression->ExpressionGUID;
											bChanged = true;
											break;
										}
									}
								}
							}

							return false;
						};

						if (UpdateParameterSetInFunctions())
						{
							bChanged = true;
							break;
						}
					}
				}
			}
		}
	}

	return bChanged;
}
