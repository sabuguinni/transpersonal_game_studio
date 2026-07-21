#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "../SharedTypes.h"
#include "Char_TribalCharacterManager.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Child       UMETA(DisplayName = "Child"),
    Elder       UMETA(DisplayName = "Elder")
};

USTRUCT(BlueprintType)
struct FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<USkeletalMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<TSoftObjectPtr<UStaticMesh>> Accessories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyScale;

    FChar_TribalAppearance()
    {
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        BodyScale = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FChar_TribalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Strength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Agility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Intelligence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Survival;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina;

    FChar_TribalStats()
    {
        Strength = 50.0f;
        Agility = 50.0f;
        Intelligence = 50.0f;
        Survival = 50.0f;
        Health = 100.0f;
        Stamina = 100.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_TribalCharacterManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_TribalCharacterManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character Generation
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void GenerateRandomTribalCharacter(EChar_TribalRole Role);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void ApplyTribalAppearance(const FChar_TribalAppearance& Appearance);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void SetTribalRole(EChar_TribalRole NewRole);

    // Character Customization
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetSkinTone(const FLinearColor& NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void AttachAccessory(UStaticMesh* Accessory, const FName& SocketName);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void RemoveAccessory(const FName& SocketName);

    // Character Stats
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void SetTribalStats(const FChar_TribalStats& NewStats);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stats")
    FChar_TribalStats GetTribalStats() const { return CurrentStats; }

    // Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateCharacterSetup() const;

    UFUNCTION(CallInEditor, Category = "Editor")
    void RegenerateCharacter();

protected:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (AllowPrivateAccess = "true"))
    EChar_TribalRole CurrentRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (AllowPrivateAccess = "true"))
    FChar_TribalAppearance CurrentAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (AllowPrivateAccess = "true"))
    FChar_TribalStats CurrentStats;

    // Component References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USkeletalMeshComponent> CharacterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<UStaticMeshComponent>> AccessoryComponents;

    // Generation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    TArray<TSoftObjectPtr<USkeletalMesh>> AvailableMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    TArray<TSoftObjectPtr<UMaterialInterface>> AvailableMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    TMap<EChar_TribalRole, FChar_TribalStats> RoleStatTemplates;

private:
    // Internal Methods
    void InitializeRoleTemplates();
    void LoadDefaultAssets();
    FChar_TribalStats GenerateStatsForRole(EChar_TribalRole Role) const;
    void ApplyRoleSpecificCustomization(EChar_TribalRole Role);
    void UpdateCharacterMesh();
    void UpdateCharacterMaterials();
    void SpawnAccessoryComponents();
    void CleanupAccessoryComponents();

    // Validation
    bool bIsInitialized;
    bool bHasValidMesh;
    bool bHasValidMaterials;
};