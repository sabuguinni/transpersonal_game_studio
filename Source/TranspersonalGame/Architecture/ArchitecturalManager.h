#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "../SharedTypes.h"
#include "ArchitecturalManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Foundation,
    Wall,
    Roof,
    Door,
    Window,
    Firepit,
    StoragePit,
    WaterBasin,
    ToolCache,
    ResourceZone
};

UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Limestone,
    Sandstone,
    Clay,
    Wood,
    Bone,
    Hide,
    Stone,
    Mud
};

UENUM(BlueprintType)
enum class EArch_ConstructionTechnique : uint8
{
    DryStone,
    MudMortar,
    WoodFrame,
    EarthWork,
    BoneReinforced,
    WattleAndDaub
};

USTRUCT(BlueprintType)
struct FArch_StructuralElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_MaterialType PrimaryMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_ConstructionTechnique Technique;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Dimensions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatherResistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 AgeInYears;

    FArch_StructuralElement()
    {
        StructureType = EArch_StructureType::Foundation;
        PrimaryMaterial = EArch_MaterialType::Stone;
        Technique = EArch_ConstructionTechnique::DryStone;
        Dimensions = FVector(100, 100, 50);
        StructuralIntegrity = 100.0f;
        WeatherResistance = 75.0f;
        AgeInYears = 0;
    }
};

USTRUCT(BlueprintType)
struct FArch_EnvironmentalStory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString StoryElement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString ArchaeologicalEvidence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FString> AssociatedArtifacts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float PreservationLevel;

    FArch_EnvironmentalStory()
    {
        StoryElement = TEXT("Unknown Settlement");
        ArchaeologicalEvidence = TEXT("Structural remains");
        PreservationLevel = 50.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructuralElement> StructuralElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_EnvironmentalStory> EnvironmentalStories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<AActor*> FoundationActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<AActor*> StorytellingActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<AActor*> ResourceZoneActors;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeArchitecturalFramework();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreateStructuralElement(EArch_StructureType Type, FVector Location, EArch_MaterialType Material);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PlaceEnvironmentalStoryElement(const FString& StoryName, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructuralIntegrity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatherDamage(float WeatherIntensity);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructuralElement> GetNearbyStructures(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArch_EnvironmentalStory AnalyzeArchaeologicalSite(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanBuildAtLocation(FVector Location, EArch_StructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterFoundationActor(AActor* FoundationActor);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterStorytellingActor(AActor* StoryActor);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterResourceZoneActor(AActor* ResourceActor);

protected:
    UFUNCTION()
    void OnStructuralElementDestroyed();

    UFUNCTION()
    void ValidateArchitecturalIntegrity();

    UFUNCTION()
    void UpdateEnvironmentalStorytelling();

private:
    float ArchitecturalUpdateInterval;
    float LastArchitecturalUpdate;
    bool bIsInitialized;
};