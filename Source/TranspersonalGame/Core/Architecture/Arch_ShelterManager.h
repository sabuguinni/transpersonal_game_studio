#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerVolume.h"
#include "Arch_ShelterManager.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    StoneArchway,
    CaveEntrance,
    RockOverhang,
    LogShelter,
    BuriedDwelling
};

USTRUCT(BlueprintType)
struct FArch_ShelterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    FVector EntranceLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    FVector InteriorBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasSleepingArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float ProtectionLevel;

    FArch_ShelterData()
    {
        ShelterType = EArch_ShelterType::StoneArchway;
        EntranceLocation = FVector::ZeroVector;
        InteriorBounds = FVector(400, 400, 300);
        bHasFirePit = false;
        bHasSleepingArea = false;
        ProtectionLevel = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ShelterManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_ShelterManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* EntranceMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteriorTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FirePitMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    FArch_ShelterData ShelterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    TArray<UStaticMesh*> EntranceMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    UStaticMesh* FirePitMesh_Asset;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void InitializeShelter(const FArch_ShelterData& NewShelterData);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void SetShelterType(EArch_ShelterType NewType);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool IsPlayerInShelter() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetProtectionLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void ToggleFirePit(bool bEnable);

protected:
    UFUNCTION()
    void OnInteriorEntered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteriorExited(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

private:
    bool bPlayerInside;
    float CurrentProtection;

    void UpdateMeshBasedOnType();
    void SetupInteriorTrigger();
};