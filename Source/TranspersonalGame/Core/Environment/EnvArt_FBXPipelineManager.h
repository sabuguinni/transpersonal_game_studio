#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"
#include "AssetImportTask.h"
#include "Factories/FbxImportUI.h"
#include "SharedTypes.h"
#include "EnvArt_FBXPipelineManager.generated.h"

/**
 * FBX Pipeline Manager for Environment Art Assets
 * Handles automated import and placement of FBX assets from external sources
 * Critical component for CRITERIO #3 - FBX pipeline validation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_FBXPipelineManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_FBXPipelineManager();

protected:
    virtual void BeginPlay() override;

    // FBX Import Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FBX Import")
    FString TestAssetsPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FBX Import")
    FString ImportDestinationPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FBX Import")
    bool bImportMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FBX Import")
    bool bImportTextures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FBX Import")
    bool bImportMaterials;

    // Biome Placement Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Placement")
    FVector SwampBiomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Placement")
    FVector ForestBiomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Placement")
    FVector SavannaBiomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Placement")
    FVector DesertBiomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Placement")
    FVector MountainBiomeLocation;

    // Asset Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Asset Management")
    TArray<UStaticMesh*> ImportedAssets;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Asset Management")
    TArray<AActor*> SpawnedActors;

public:
    // FBX Import Functions
    UFUNCTION(BlueprintCallable, Category = "FBX Import")
    bool ImportFBXAsset(const FString& FilePath, const FString& AssetName);

    UFUNCTION(BlueprintCallable, Category = "FBX Import")
    UAssetImportTask* CreateImportTask(const FString& FilePath, const FString& DestinationPath);

    UFUNCTION(BlueprintCallable, Category = "FBX Import")
    UFbxImportUI* ConfigureFBXImportOptions();

    // Asset Discovery
    UFUNCTION(BlueprintCallable, Category = "Asset Management")
    TArray<FString> ScanForFBXFiles();

    UFUNCTION(BlueprintCallable, Category = "Asset Management")
    bool ValidateAssetImport(const FString& AssetPath);

    // Biome Placement
    UFUNCTION(BlueprintCallable, Category = "Biome Placement")
    AActor* SpawnAssetInBiome(UStaticMesh* Asset, EBiomeType BiomeType, const FVector& Offset = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Biome Placement")
    FVector GetBiomeSpawnLocation(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Placement")
    void PlaceAssetCluster(UStaticMesh* Asset, EBiomeType BiomeType, int32 Count, float Radius);

    // Pipeline Validation
    UFUNCTION(BlueprintCallable, Category = "Pipeline Validation")
    bool ValidateFBXPipeline();

    UFUNCTION(BlueprintCallable, Category = "Pipeline Validation")
    void GeneratePipelineReport();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void CleanupTestAssets();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void RefreshAssetRegistry();

private:
    // Internal pipeline state
    bool bPipelineInitialized;
    FDateTime LastImportTime;
    int32 SuccessfulImports;
    int32 FailedImports;

    // Helper functions
    void InitializeBiomeLocations();
    bool EnsureDirectoryExists(const FString& DirectoryPath);
    void LogImportResult(const FString& AssetName, bool bSuccess, const FString& ErrorMessage = TEXT(""));
};