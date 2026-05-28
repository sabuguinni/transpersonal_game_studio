#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Elder       UMETA(DisplayName = "Elder")
};

USTRUCT(BlueprintType)
struct FChar_TribalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Stats")
    float Strength = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Stats")
    float Agility = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Stats")
    float Wisdom = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Stats")
    float Survival = 50.0f;

    FChar_TribalStats()
    {
        Strength = 50.0f;
        Agility = 50.0f;
        Wisdom = 50.0f;
        Survival = 50.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATribalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATribalCharacter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    EChar_TribalRole TribalRole = EChar_TribalRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FChar_TribalStats TribalStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FString CharacterName = TEXT("Unnamed Tribal");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    bool bIsMale = true;

    // Equipment components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    UStaticMeshComponent* WeaponMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    UStaticMeshComponent* ToolMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor TribalMarkingColor = FLinearColor(0.2f, 0.1f, 0.0f, 1.0f);

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    EChar_TribalRole GetTribalRole() const { return TribalRole; }

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetCharacterName(const FString& NewName);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    FString GetCharacterName() const { return CharacterName; }

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void ApplyTribalMarkings();

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void EquipWeapon(UStaticMesh* WeaponMeshAsset);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void EquipTool(UStaticMesh* ToolMeshAsset);

    UFUNCTION(BlueprintPure, Category = "Tribal Character")
    float GetStatValue(const FString& StatName) const;

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void ModifyStatValue(const FString& StatName, float Delta);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Tribal Character")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Tribal Character")
    void SetupDefaultEquipment();
};

#include "TribalCharacter.generated.h"#include "TribalCharacter.generated.h"
