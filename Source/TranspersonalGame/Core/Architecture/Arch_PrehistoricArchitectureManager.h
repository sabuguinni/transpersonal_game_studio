#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Arch_PrehistoricArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_MonumentType : uint8
{
    StoneCircle         UMETA(DisplayName = "Stone Circle"),
    MegalithicDolmen    UMETA(DisplayName = "Megalithic Dolmen"),
    CairnPyramid        UMETA(DisplayName = "Cairn Pyramid"),
    StandingStones      UMETA(DisplayName = "Standing Stones"),
    RockFormation       UMETA(DisplayName = "Natural Rock Formation")
};

USTRUCT(BlueprintType)
struct FArch_MonumentProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monument")
    EArch_MonumentType MonumentType = EArch_MonumentType::StoneCircle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monument")
    float CircleRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monument")
    int32 StoneCount = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monument")
    FVector StoneScale = FVector(1.0f, 1.0f, 2.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monument")
    float WeatheringIntensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monument")
    bool bHasMossGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monument")
    bool bIsAncientRuin = false;

    FArch_MonumentProperties()
    {
        MonumentType = EArch_MonumentType::StoneCircle;
        CircleRadius = 500.0f;
        StoneCount = 12;
        StoneScale = FVector(1.0f, 1.0f, 2.0f);
        WeatheringIntensity = 0.7f;
        bHasMossGrowth = true;
        bIsAncientRuin = false;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AArch_PrehistoricArchitectureManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_PrehistoricArchitectureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture System")
    TArray<FArch_MonumentProperties> ActiveMonuments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture System")
    float MonumentSpawnRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture System")
    int32 MaxMonumentsPerBiome = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WeatheredStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* MossyStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* StandingStoneBaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* DolmenCapMesh;

public:
    UFUNCTION(BlueprintCallable, Category = "Architecture System")
    void GenerateMonumentAtLocation(const FVector& Location, EArch_MonumentType MonumentType);

    UFUNCTION(BlueprintCallable, Category = "Architecture System")
    void CreateStoneCircle(const FVector& CenterLocation, const FArch_MonumentProperties& Properties);

    UFUNCTION(BlueprintCallable, Category = "Architecture System")
    void CreateMegalithicDolmen(const FVector& Location, const FArch_MonumentProperties& Properties);

    UFUNCTION(BlueprintCallable, Category = "Architecture System")
    void CreateCairnPyramid(const FVector& Location, const FArch_MonumentProperties& Properties);

    UFUNCTION(BlueprintCallable, Category = "Architecture System")
    void ApplyWeatheringEffects(UStaticMeshComponent* MeshComponent, float WeatheringIntensity);

    UFUNCTION(BlueprintCallable, Category = "Architecture System")
    void RegisterMonument(const FArch_MonumentProperties& Monument);

    UFUNCTION(BlueprintCallable, Category = "Architecture System")
    TArray<FArch_MonumentProperties> GetNearbyMonuments(const FVector& Location, float SearchRadius) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture System")
    void GenerateTestMonuments();

private:
    void InitializeDefaultAssets();
    UStaticMeshComponent* CreateStoneComponent(const FVector& Location, const FVector& Scale, float RotationYaw);
    void ApplyMaterialToComponent(UStaticMeshComponent* Component, bool bUseMossyVariant);
};