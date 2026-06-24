#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StonePillar     UMETA(DisplayName = "Stone Pillar"),
    RuinWall        UMETA(DisplayName = "Ruin Wall"),
    RockOutcrop     UMETA(DisplayName = "Rock Outcrop"),
    AncientArchway  UMETA(DisplayName = "Ancient Archway"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_StructureType StructureType = EArch_StructureType::StonePillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString AssetLabel = TEXT("Arch_Prop");
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitectureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitectureManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> RegisteredStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 TotalStructuresSpawned = 0;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterStructure(const FArch_StructureData& StructureData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    int32 GetStructureCount() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearAllStructures();

protected:
    virtual void BeginPlay() override;
};
