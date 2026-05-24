#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/TriggerVolume.h"
#include "Arch_PrimitiveStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None = 0,
    Cave = 1,
    WoodenShelter = 2,
    StoneCircle = 3,
    CliffDwelling = 4,
    RockFormation = 5
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float InteriorRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasInteriorLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsHabitable = true;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::None;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        InteriorRadius = 500.0f;
        bHasInteriorLighting = true;
        bIsHabitable = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_PrimitiveStructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_PrimitiveStructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
    TArray<FArch_StructureData> RegisteredStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxStructuresPerBiome = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MinDistanceBetweenStructures = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAutoGenerateStructures = true;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateStructuresInBiome(const FVector& BiomeCenter, float BiomeRadius, EArch_StructureType PreferredType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AActor* CreatePrimitiveStructure(EArch_StructureType StructureType, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetupInteriorLighting(AActor* StructureActor, EArch_StructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreateInteriorTriggerZone(AActor* StructureActor, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AActor*> GetNearbyStructures(const FVector& Location, float SearchRadius) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsLocationSuitableForStructure(const FVector& Location, EArch_StructureType StructureType) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterExistingStructure(AActor* StructureActor, EArch_StructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearAllStructures();

    UFUNCTION(BlueprintPure, Category = "Architecture")
    int32 GetStructureCount() const { return RegisteredStructures.Num(); }

    UFUNCTION(BlueprintPure, Category = "Architecture")
    FArch_StructureData GetStructureData(int32 Index) const;

protected:
    UFUNCTION()
    void OnStructureEntered(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION()
    void OnStructureExited(AActor* OverlappedActor, AActor* OtherActor);

private:
    void InitializeStructureGeneration();
    FVector FindSuitableStructureLocation(const FVector& BiomeCenter, float BiomeRadius, EArch_StructureType StructureType);
    void ApplyStructureSpecificSettings(AActor* StructureActor, EArch_StructureType StructureType);
};