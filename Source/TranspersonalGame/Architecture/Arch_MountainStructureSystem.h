#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Arch_MountainStructureSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_MountainStructureType : uint8
{
    StoneArch       UMETA(DisplayName = "Stone Arch"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    RockFormation   UMETA(DisplayName = "Rock Formation"),
    CliffDwelling   UMETA(DisplayName = "Cliff Dwelling"),
    StonePlatform   UMETA(DisplayName = "Stone Platform")
};

USTRUCT(BlueprintType)
struct FArch_MountainStructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Structure")
    EArch_MountainStructureType StructureType = EArch_MountainStructureType::StoneArch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Structure")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Structure")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Structure")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Structure")
    bool bHasMossGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Structure")
    bool bHasLichenCoverage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Structure")
    float StructuralIntegrity = 1.0f;

    FArch_MountainStructureData()
    {
        StructureType = EArch_MountainStructureType::StoneArch;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        WeatheringLevel = 0.5f;
        bHasMossGrowth = true;
        bHasLichenCoverage = true;
        StructuralIntegrity = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_MountainStructureSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_MountainStructureSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Mountain Structure Management
    UFUNCTION(BlueprintCallable, Category = "Mountain Architecture")
    void CreateMountainStructure(const FArch_MountainStructureData& StructureData);

    UFUNCTION(BlueprintCallable, Category = "Mountain Architecture")
    void RemoveMountainStructure(int32 StructureIndex);

    UFUNCTION(BlueprintCallable, Category = "Mountain Architecture")
    void UpdateStructureWeathering(int32 StructureIndex, float NewWeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Mountain Architecture")
    void ApplyMossGrowth(int32 StructureIndex, bool bEnableMoss);

    UFUNCTION(BlueprintCallable, Category = "Mountain Architecture")
    void ApplyLichenCoverage(int32 StructureIndex, bool bEnableLichen);

    // Structure Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mountain Architecture")
    int32 GetStructureCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mountain Architecture")
    FArch_MountainStructureData GetStructureData(int32 StructureIndex) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mountain Architecture")
    TArray<int32> FindStructuresInRadius(FVector Center, float Radius) const;

    // Environmental Integration
    UFUNCTION(BlueprintCallable, Category = "Mountain Architecture")
    void IntegrateWithTerrain(int32 StructureIndex);

    UFUNCTION(BlueprintCallable, Category = "Mountain Architecture")
    void ApplyWeatherEffects(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Mountain Architecture")
    void UpdateStructuralIntegrity(int32 StructureIndex, float IntegrityChange);

    // Material Management
    UFUNCTION(BlueprintCallable, Category = "Mountain Architecture")
    void SetStructureMaterial(int32 StructureIndex, UMaterialInterface* NewMaterial);

    UFUNCTION(BlueprintCallable, Category = "Mountain Architecture")
    void ApplyWeatheringMaterial(int32 StructureIndex);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Structures", meta = (AllowPrivateAccess = "true"))
    TArray<FArch_MountainStructureData> MountainStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Structures", meta = (AllowPrivateAccess = "true"))
    TArray<AStaticMeshActor*> StructureActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
    UMaterialInterface* BaseStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
    UMaterialInterface* WeatheredStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
    UMaterialInterface* MossyCoverMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
    UMaterialInterface* LichenCoverMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects", meta = (AllowPrivateAccess = "true"))
    float WeatheringRate = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects", meta = (AllowPrivateAccess = "true"))
    float MossGrowthRate = 0.005f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects", meta = (AllowPrivateAccess = "true"))
    float LichenGrowthRate = 0.003f;

private:
    void InitializeStructureActor(AStaticMeshActor* StructureActor, const FArch_MountainStructureData& StructureData);
    void UpdateStructureAppearance(int32 StructureIndex);
    UMaterialInterface* GetAppropriateStructureMaterial(const FArch_MountainStructureData& StructureData) const;
    void ApplyEnvironmentalEffects(int32 StructureIndex, float DeltaTime);
};