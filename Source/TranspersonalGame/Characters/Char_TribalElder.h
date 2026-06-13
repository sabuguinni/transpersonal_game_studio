#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_TribalElder.generated.h"

/**
 * Tribal elder character representing wisdom and leadership
 * Older human survivor with extensive knowledge of the prehistoric world
 * Features weathered appearance, ceremonial items, and elder markings
 * Serves as quest giver and lore keeper in the game world
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalElder : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_TribalElder();

protected:
    virtual void BeginPlay() override;

    // Elder appearance components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance")
    class UStaticMeshComponent* StaffMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance")
    class UStaticMeshComponent* FeatherHeaddressMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance")
    class UStaticMeshComponent* BoneCharmsMesh;

    // Elder customization properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UMaterialInterface* AgedSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UMaterialInterface* CeremonialRobesMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UStaticMesh* WoodenStaffAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UStaticMesh* FeatherHeaddressAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UStaticMesh* BoneCharmsAsset;

    // Elder wisdom stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisdom")
    float KnowledgeLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisdom")
    float DinosaurLore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisdom")
    float SurvivalWisdom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisdom")
    int32 YearsOfExperience;

public:
    virtual void Tick(float DeltaTime) override;

    // Elder setup functions
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void SetupElderAppearance();

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void AttachCeremonialItems();

    UFUNCTION(BlueprintCallable, Category = "Wisdom")
    void SetWisdomStats(float Knowledge, float DinoLore, float Wisdom, int32 Years);

    UFUNCTION(BlueprintPure, Category = "Wisdom")
    float GetKnowledgeLevel() const { return KnowledgeLevel; }

    UFUNCTION(BlueprintPure, Category = "Wisdom")
    float GetDinosaurLore() const { return DinosaurLore; }

    UFUNCTION(BlueprintPure, Category = "Wisdom")
    float GetSurvivalWisdom() const { return SurvivalWisdom; }

    UFUNCTION(BlueprintPure, Category = "Wisdom")
    int32 GetYearsOfExperience() const { return YearsOfExperience; }

    // Elder interaction functions
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    FString GetWisdomQuote();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool CanShareKnowledge() const;
};