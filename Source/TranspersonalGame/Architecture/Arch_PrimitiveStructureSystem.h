#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "Arch_PrimitiveStructureSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_PrimitiveStructureType : uint8
{
    StoneArchway        UMETA(DisplayName = "Stone Archway"),
    CaveEntrance        UMETA(DisplayName = "Cave Entrance"),
    RockOverhang        UMETA(DisplayName = "Rock Overhang"),
    StonePlatform       UMETA(DisplayName = "Stone Platform"),
    NaturalBridge       UMETA(DisplayName = "Natural Bridge"),
    WeatheredPillar     UMETA(DisplayName = "Weathered Pillar")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_PrimitiveStructureType StructureType = EArch_PrimitiveStructureType::StoneArchway;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasToolMarks = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Dimensions = FVector(400.0f, 200.0f, 300.0f);

    FArch_StructureProperties()
    {
        StructureType = EArch_PrimitiveStructureType::StoneArchway;
        WeatheringLevel = 0.5f;
        bHasMossGrowth = true;
        bHasToolMarks = false;
        StructuralIntegrity = 1.0f;
        Dimensions = FVector(400.0f, 200.0f, 300.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_PrimitiveStructureActor : public AActor
{
    GENERATED_BODY()

public:
    AArch_PrimitiveStructureActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Properties")
    FArch_StructureProperties StructureConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bProvidesWeatherProtection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float ProtectionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* AmbientSound;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetStructureType(EArch_PrimitiveStructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    bool IsPlayerInProtectedArea(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    float GetWeatherProtectionStrength() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure")
    void OnPlayerEnterStructure();

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure")
    void OnPlayerExitStructure();

protected:
    UFUNCTION()
    void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void UpdateStructureMesh();
    void ConfigureCollisionAndPhysics();
    
    bool bPlayerInside = false;
    float LastWeatherCheckTime = 0.0f;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UArch_PrimitiveStructureLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor = true)
    static TArray<AArch_PrimitiveStructureActor*> FindNearbyStructures(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    static AArch_PrimitiveStructureActor* GetClosestStructure(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    static bool IsLocationProtectedByStructure(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor = true)
    static void SpawnStructureCluster(const FVector& CenterLocation, int32 Count, float SpreadRadius);
};