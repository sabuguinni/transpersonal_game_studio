#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "PropPlacementSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvironmentPropType : uint8
{
    Rock_Small         UMETA(DisplayName = "Small Rock"),
    Rock_Medium        UMETA(DisplayName = "Medium Rock"),
    Rock_Large         UMETA(DisplayName = "Large Boulder"),
    Log_Fallen         UMETA(DisplayName = "Fallen Log"),
    Bone_Skull         UMETA(DisplayName = "Dinosaur Skull"),
    Bone_Ribcage       UMETA(DisplayName = "Ribcage"),
    Crystal_Small      UMETA(DisplayName = "Small Crystal"),
    Crystal_Large      UMETA(DisplayName = "Large Crystal Formation"),
    Mud_Pool           UMETA(DisplayName = "Mud Pool"),
    Stone_Formation    UMETA(DisplayName = "Stone Formation")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvironmentPropData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    EEnvironmentPropType PropType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    TSoftObjectPtr<UStaticMesh> PropMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    TSoftObjectPtr<UMaterialInterface> PropMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    float MinScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    float MaxScale = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    float SpawnWeight = 1.0f; // Probability weight for spawning

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    bool bCanRotateRandomly = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    bool bAlignToSurface = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    bool bNaniteEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    float StorytellingValue = 0.0f; // How much this prop contributes to environmental storytelling

    FEnvironmentPropData()
    {
        PropType = EEnvironmentPropType::Rock_Small;
        MinScale = 0.8f;
        MaxScale = 1.2f;
        SpawnWeight = 1.0f;
        bCanRotateRandomly = true;
        bAlignToSurface = true;
        bNaniteEnabled = true;
        StorytellingValue = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FStorytellingCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString ClusterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<EEnvironmentPropType> RequiredProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float ClusterRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString StoryHint; // What story this cluster tells

    FStorytellingCluster()
    {
        ClusterName = TEXT("Generic Cluster");
        ClusterRadius = 500.0f;
        StoryHint = TEXT("Something happened here...");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APropPlacementSystem : public AActor
{
    GENERATED_BODY()

public:
    APropPlacementSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Props")
    TArray<FEnvironmentPropData> EnvironmentProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FStorytellingCluster> StorytellingClusters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement Settings")
    float PropDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement Settings")
    float MinDistanceBetweenProps = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement Settings")
    bool bEnableStorytellingClusters = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement Settings")
    float StorytellingClusterChance = 0.15f; // 15% chance per area

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RootSceneComponent;

public:
    UFUNCTION(BlueprintCallable, Category = "Prop Placement")
    void PlacePropsInArea(FVector Center, float Radius, const FString& BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Prop Placement")
    void CreateStorytellingCluster(FVector Location, const FStorytellingCluster& ClusterData);

    UFUNCTION(BlueprintCallable, Category = "Prop Placement")
    void ClearPropsInArea(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Prop Placement")
    void UpdatePropDensity(float NewDensity);

    UFUNCTION(BlueprintImplementableEvent, Category = "Prop Events")
    void OnStorytellingClusterCreated(const FString& ClusterName, FVector Location);

private:
    UPROPERTY()
    TArray<TObjectPtr<AActor>> SpawnedProps;

    void InitializePropTypes();
    void InitializeStorytellingClusters();
    AActor* SpawnProp(const FEnvironmentPropData& PropData, FVector Location, FRotator Rotation, FVector Scale);
    TArray<FEnvironmentPropData> GetPropsForBiome(const FString& BiomeType) const;
    bool IsLocationValid(FVector Location, float MinDistance) const;
    FVector GetSurfaceLocation(FVector Location) const;
    FRotator GetSurfaceRotation(FVector Location) const;
};