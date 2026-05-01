#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerSphere.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Combat_DamageSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_DamageType : uint8
{
    None            UMETA(DisplayName = "No Damage"),
    Bite            UMETA(DisplayName = "Bite Attack"),
    Claw            UMETA(DisplayName = "Claw Strike"),
    Tail            UMETA(DisplayName = "Tail Whip"),
    Charge          UMETA(DisplayName = "Charge Attack"),
    Environmental   UMETA(DisplayName = "Environmental")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DamageInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float DamageAmount = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    ECombat_DamageType DamageType = ECombat_DamageType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector ImpactDirection = FVector::ForwardVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    AActor* DamageSource = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bCanCauseFear = true;

    FCombat_DamageInfo()
    {
        DamageAmount = 25.0f;
        DamageType = ECombat_DamageType::Bite;
        ImpactLocation = FVector::ZeroVector;
        ImpactDirection = FVector::ForwardVector;
        DamageSource = nullptr;
        bCanCauseFear = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DamageZone : public ATriggerSphere
{
    GENERATED_BODY()

public:
    ACombat_DamageZone();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_DamageInfo DamageInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DamageInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bContinuousDamage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bOnlyDamagePlayer = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* OwnerDinosaur = nullptr;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                       bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    UPROPERTY()
    TArray<AActor*> ActorsInZone;

    FTimerHandle DamageTimerHandle;

    UFUNCTION()
    void ApplyContinuousDamage();

public:
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetDamageInfo(const FCombat_DamageInfo& NewDamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetOwnerDinosaur(AActor* NewOwner);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ActivateDamageZone(bool bActivate);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_DamageSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamage(AActor* Target, const FCombat_DamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterDamageZone(ACombat_DamageZone* DamageZone);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UnregisterDamageZone(ACombat_DamageZone* DamageZone);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TArray<ACombat_DamageZone*> GetActiveDamageZones() const;

protected:
    UPROPERTY()
    TArray<ACombat_DamageZone*> ActiveDamageZones;

    void ProcessDamageToCharacter(ACharacter* Character, const FCombat_DamageInfo& DamageInfo);
    void ApplyFearEffect(ACharacter* Character, const FCombat_DamageInfo& DamageInfo);
};

#include "Combat_DamageSystem.generated.h"