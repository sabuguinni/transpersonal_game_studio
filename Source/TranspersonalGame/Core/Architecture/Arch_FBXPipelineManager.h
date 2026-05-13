#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "AssetImportTask.h"
#include "Factories/FbxImportUI.h"
#include "EditorAssetLibrary.h"
#include "Arch_DwellingTypes.h"
#include "Arch_FBXPipelineManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_FBXAssetInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FBX Asset")
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FBX Asset")
    FString SourceFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FBX Asset")
    FString DestinationPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FBX Asset")
    EArch_DwellingType DwellingType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FBX Asset")
    FVector PlacementLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FBX Asset")
    FRotator PlacementRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FBX Asset")
    bool bImportMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FBX Asset")
    bool bImportTextures;

    FArch_FBXAssetInfo()
    {
        AssetName = TEXT("");
        SourceFilePath = TEXT("");
        DestinationPath = TEXT("/Game/TranspersonalGame/Architecture/");
        DwellingType = EArch_DwellingType::Cave;
        PlacementLocation = FVector::ZeroVector;
        PlacementRotation = FRotator::ZeroRotator;
        bImportMaterials = true;
        bImportTextures = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ImportResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Import Result")
    bool bSuccess;

    UPROPERTY(BlueprintReadOnly, Category = "Import Result")
    FString ImportedAssetPath;

    UPROPERTY(BlueprintReadOnly, Category = "Import Result")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Import Result")
    TObjectPtr<UStaticMesh> ImportedMesh;

    FArch_ImportResult()
    {
        bSuccess = false;
        ImportedAssetPath = TEXT("");
        ErrorMessage = TEXT("");
        ImportedMesh = nullptr;
    }
};

/**
 * FBX Pipeline Manager for Architecture Agent
 * Handles import and placement of architectural FBX assets from Fab.com and other sources
 * Implements the mandatory FBX pipeline testing workflow
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArch_FBXPipelineManager : public UObject
{
    GENERATED_BODY()

public:
    UArch_FBXPipelineManager();

    // Core FBX import functionality
    UFUNCTION(BlueprintCallable, Category = "Architecture FBX Pipeline")
    FArch_ImportResult ImportFBXAsset(const FArch_FBXAssetInfo& AssetInfo);

    UFUNCTION(BlueprintCallable, Category = "Architecture FBX Pipeline")
    bool ConfigureImportTask(UAssetImportTask* ImportTask, const FArch_FBXAssetInfo& AssetInfo);

    UFUNCTION(BlueprintCallable, Category = "Architecture FBX Pipeline")
    AStaticMeshActor* PlaceImportedAsset(UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation);

    // Batch import for multiple assets
    UFUNCTION(BlueprintCallable, Category = "Architecture FBX Pipeline")
    TArray<FArch_ImportResult> BatchImportFBXAssets(const TArray<FArch_FBXAssetInfo>& AssetList);

    // Validation and testing
    UFUNCTION(BlueprintCallable, Category = "Architecture FBX Pipeline")
    bool ValidateFBXPipeline();

    UFUNCTION(BlueprintCallable, Category = "Architecture FBX Pipeline")
    bool TestWithFreeAsset(const FString& FreeAssetPath);

    // Asset management
    UFUNCTION(BlueprintCallable, Category = "Architecture FBX Pipeline")
    TArray<FString> GetAvailableArchitecturalAssets();

    UFUNCTION(BlueprintCallable, Category = "Architecture FBX Pipeline")
    bool CleanupTestAssets();

protected:
    // Predefined placement locations for different dwelling types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    TMap<EArch_DwellingType, FVector> DefaultPlacementLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    FString TestAssetsDirectory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    FString ArchitectureAssetsDirectory;

    // Import statistics
    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 TotalImportsAttempted;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 SuccessfulImports;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 FailedImports;

private:
    void InitializeDefaultLocations();
    bool ValidateAssetPath(const FString& AssetPath);
    FString GenerateUniqueAssetName(const FString& BaseName);
    void LogImportResult(const FArch_ImportResult& Result);
};