#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerVolume.h"
#include "Arch_StructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Dwelling,
    Ruins,
    Shelter,
    Storage,
    Defensive
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Integrity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOccupied;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> InteriorItems;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::Dwelling;
        Location = FVector::ZeroVector;
        Integrity = 100.0f;
        bIsOccupied = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_StructureManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_StructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> RegisteredStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float StructureDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoDetectStructures;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterStructure(const FArch_StructureData& StructureData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UnregisterStructure(int32 StructureIndex);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructureData> GetNearbyStructures(FVector PlayerLocation, float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsInsideStructure(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructureIntegrity(int32 StructureIndex, float NewIntegrity);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureOccupancy(int32 StructureIndex, bool bOccupied);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArch_StructureData GetStructureAtLocation(FVector Location, float Tolerance = 100.0f);
};