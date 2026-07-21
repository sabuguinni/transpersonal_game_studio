#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "ArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    None            UMETA(DisplayName = "None"),
    Cave            UMETA(DisplayName = "Cave Dwelling"),
    WoodenShelter   UMETA(DisplayName = "Wooden Shelter"),
    TreePlatform    UMETA(DisplayName = "Tree Platform"),
    StoneCircle     UMETA(DisplayName = "Stone Circle"),
    RockOverhang    UMETA(DisplayName = "Rock Overhang")
};

USTRUCT(BlueprintType)
struct FArch_ShelterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float InteriorRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasCampfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float SafetyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants;

    FArch_ShelterData()
    {
        ShelterType = EArch_ShelterType::None;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        InteriorRadius = 500.0f;
        bHasCampfire = false;
        SafetyLevel = 0.5f;
        MaxOccupants = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_PrimitiveShelter : public AActor
{
    GENERATED_BODY()

public:
    AArch_PrimitiveShelter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShelterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteriorTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* CampfireLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    FArch_ShelterData ShelterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bIsOccupied;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    TArray<AActor*> CurrentOccupants;

public:
    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void SetShelterType(EArch_ShelterType NewType);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanEnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void EnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void ExitShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetSafetyLevel() const { return ShelterData.SafetyLevel; }

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool HasCampfire() const { return ShelterData.bHasCampfire; }

protected:
    UFUNCTION()
    void OnInteriorEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteriorExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void UpdateCampfireLight();
    void ConfigureShelterMesh();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArch_ArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    UPROPERTY()
    TArray<AArch_PrimitiveShelter*> RegisteredShelters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TMap<EArch_ShelterType, UStaticMesh*> ShelterMeshes;

public:
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AArch_PrimitiveShelter* CreateShelter(EArch_ShelterType ShelterType, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterShelter(AArch_PrimitiveShelter* Shelter);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UnregisterShelter(AArch_PrimitiveShelter* Shelter);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AArch_PrimitiveShelter*> GetNearbyShelters(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AArch_PrimitiveShelter* FindClosestShelter(FVector Location, EArch_ShelterType ShelterType = EArch_ShelterType::None);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void LoadDefaultShelterMeshes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void SpawnTestShelters();

private:
    void SetupShelterDefaults();
};