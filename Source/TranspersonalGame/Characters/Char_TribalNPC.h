#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMeshActor.h"
#include "../SharedTypes.h"
#include "Char_TribalNPC.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Child       UMETA(DisplayName = "Child")
};

USTRUCT(BlueprintType)
struct FChar_TribalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Thirst = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Morale = 75.0f;

    FChar_TribalStats()
    {
        Health = 100.0f;
        Stamina = 100.0f;
        Hunger = 50.0f;
        Thirst = 50.0f;
        Morale = 75.0f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AChar_TribalNPC : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_TribalNPC();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    EChar_TribalRole TribalRole = EChar_TribalRole::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FChar_TribalStats TribalStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FString CharacterName = TEXT("Tribal Member");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    bool bIsMale = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    float WalkSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    float RunSpeed = 400.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintPure, Category = "Tribal Character")
    EChar_TribalRole GetTribalRole() const { return TribalRole; }

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void ModifyHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void ModifyStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void ModifyHunger(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void ModifyThirst(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void ModifyMorale(float Amount);

    UFUNCTION(BlueprintPure, Category = "Tribal Character")
    FChar_TribalStats GetTribalStats() const { return TribalStats; }

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetCharacterAppearance(bool bMale, FLinearColor NewSkinTone, int32 NewAge);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetMovementSpeeds(float NewWalkSpeed, float NewRunSpeed);

protected:
    void UpdateStatsOverTime(float DeltaTime);
    void ApplyRoleBasedBehavior();
    void ConfigureMeshForRole();
};