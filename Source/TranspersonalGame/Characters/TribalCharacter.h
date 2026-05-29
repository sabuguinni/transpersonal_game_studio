#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "TribalCharacter.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Elder       UMETA(DisplayName = "Elder"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Scout       UMETA(DisplayName = "Scout")
};

USTRUCT(BlueprintType)
struct FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float SkinTone = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor WarPaintColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTribalMarkings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasFeatherHeaddress = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasBoneAccessories = true;

    FChar_TribalAppearance()
    {
        SkinTone = 0.5f;
        WarPaintColor = FLinearColor::Red;
        bHasTribalMarkings = true;
        bHasFeatherHeaddress = false;
        bHasBoneAccessories = true;
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

public:
    virtual void Tick(float DeltaTime) override;

    // Tribal character properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    EChar_TribalRole TribalRole = EChar_TribalRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FChar_TribalAppearance Appearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FString CharacterName = TEXT("Tribal Member");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    float ExperienceLevel = 1.0f;

    // Equipment components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    class UStaticMeshComponent* WeaponMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    class UStaticMeshComponent* AccessoryMesh;

    // Interaction system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanInteract = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionPrompt = TEXT("Talk to tribal member");

    // Character behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsHostile = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsFriendly = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float TrustLevel = 0.5f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void ApplyTribalAppearance(const FChar_TribalAppearance& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void EquipWeapon(class UStaticMesh* WeaponMeshAsset);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void EquipAccessory(class UStaticMesh* AccessoryMeshAsset);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool CanPlayerInteract(class APawn* PlayerPawn) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnPlayerInteract(class APawn* PlayerPawn);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ModifyTrustLevel(float DeltaTrust);

protected:
    // Internal functions
    void SetupDefaultEquipment();
    void ConfigureAppearanceByRole();
    void UpdateMaterialParameters();

private:
    // Cache for performance
    UPROPERTY()
    class UMaterialInstanceDynamic* DynamicSkinMaterial;

    UPROPERTY()
    class UMaterialInstanceDynamic* DynamicClothingMaterial;
};