#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Char_TribalCharacterManager.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalGender : uint8
{
    Male        UMETA(DisplayName = "Male"),
    Female      UMETA(DisplayName = "Female")
};

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Crafter     UMETA(DisplayName = "Crafter")
};

USTRUCT(BlueprintType)
struct FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float MuscleMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyFat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTribalScars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTattoos;

    FChar_TribalAppearance()
    {
        SkinTone = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
        HairColor = FLinearColor(0.1f, 0.05f, 0.0f, 1.0f);
        MuscleMass = 0.7f;
        BodyFat = 0.2f;
        Height = 1.0f;
        bHasTribalScars = true;
        bHasTattoos = false;
    }
};

USTRUCT(BlueprintType)
struct FChar_TribalEquipment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<USkeletalMesh> PrimaryWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<USkeletalMesh> SecondaryWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<USkeletalMesh> ArmorSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<USkeletalMesh> Accessories;

    FChar_TribalEquipment()
    {
        PrimaryWeapon = nullptr;
        SecondaryWeapon = nullptr;
        ArmorSet = nullptr;
        Accessories = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalCharacterManager : public AActor
{
    GENERATED_BODY()

public:
    AChar_TribalCharacterManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* CharacterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCapsuleComponent* CapsuleComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
    EChar_TribalGender Gender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
    EChar_TribalRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
    FChar_TribalAppearance Appearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
    FChar_TribalEquipment Equipment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
    int32 Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
    float ExperienceLevel;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void GenerateRandomTribalCharacter();

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void ApplyAppearanceToMesh();

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void EquipWeapon(USkeletalMesh* WeaponMesh, bool bIsPrimary = true);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void SetGender(EChar_TribalGender NewGender);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    FString GetCharacterDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void SpawnAtLocation(FVector Location, FRotator Rotation = FRotator::ZeroRotator);

protected:
    UFUNCTION()
    void RandomizeAppearance();

    UFUNCTION()
    void RandomizeEquipment();

    UFUNCTION()
    FString GenerateTribalName();

    UFUNCTION()
    void UpdateMeshBasedOnGender();

    UFUNCTION()
    void ApplyTribalMaterials();
};

#include "Char_TribalCharacterManager.generated.h"