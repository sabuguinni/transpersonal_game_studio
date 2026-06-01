#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Char_TribalNPC.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Elder       UMETA(DisplayName = "Elder"),
    Warrior     UMETA(DisplayName = "Warrior")
};

USTRUCT(BlueprintType)
struct FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height;

    FChar_TribalAppearance()
    {
        SkinTone = 0.5f;
        HairColor = FLinearColor::Black;
        EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
        BodyWeight = 0.5f;
        Height = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalNPC : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_TribalNPC();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    EChar_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    int32 Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FChar_TribalAppearance Appearance;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    UStaticMeshComponent* ToolMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    UStaticMeshComponent* ClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fatigue;

    UFUNCTION(BlueprintCallable, Category = "Tribal NPC")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Tribal NPC")
    void UpdateAppearance(const FChar_TribalAppearance& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Tribal NPC")
    void EquipTool(UStaticMesh* ToolMesh);

    UFUNCTION(BlueprintCallable, Category = "Tribal NPC")
    FString GetRoleDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool CanInteractWithPlayer() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnPlayerInteraction();

protected:
    UFUNCTION()
    void InitializeTribalNPC();

    UFUNCTION()
    void SetupRoleSpecificEquipment();

    UFUNCTION()
    void ApplyAppearanceSettings();
};