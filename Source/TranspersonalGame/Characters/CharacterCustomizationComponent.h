#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "CharacterTypes.h"
#include "CharacterCustomizationComponent.generated.h"

/**
 * Component responsible for character visual customization
 * Handles MetaHuman integration, clothing, accessories, and tribal markings
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterCustomizationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterCustomizationComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character customization data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    FChar_CharacterCustomization CustomizationData;

    // Physical traits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Traits")
    FChar_PhysicalTraits PhysicalTraits;

    // Equipment items
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<FChar_EquipmentItem> EquippedItems;

    // Material instances for dynamic customization
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<UMaterialInstanceDynamic*> DynamicMaterials;

    // Equipment mesh components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    TArray<UStaticMeshComponent*> EquipmentComponents;

    // Main character mesh reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
    USkeletalMeshComponent* CharacterMesh;

    // Customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyCustomization(const FChar_CharacterCustomization& NewCustomization);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetArchetype(EChar_CharacterArchetype NewArchetype);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(FLinearColor NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetHairColor(FLinearColor NewHairColor);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetAge(float NewAge);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetHeight(float NewHeight);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetBuild(float NewBuild);

    // Equipment functions
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void EquipItem(const FChar_EquipmentItem& Item);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void UnequipItem(EChar_EquipmentSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void ClearAllEquipment();

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    FChar_EquipmentItem GetEquippedItem(EChar_EquipmentSlot Slot) const;

    // Tribal markings and tattoos
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void AddTribalMarking(const FString& MarkingName);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void RemoveTribalMarking(const FString& MarkingName);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ClearTribalMarkings();

    // Preset application
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyArchetypePreset(EChar_CharacterArchetype Archetype);

    // Material management
    UFUNCTION(BlueprintCallable, Category = "Materials")
    void RefreshMaterials();

    UFUNCTION(BlueprintCallable, Category = "Materials")
    void UpdateMaterialParameters();

    // Validation
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    bool ValidateCustomization() const;

protected:
    // Internal helper functions
    void InitializeCharacterMesh();
    void CreateDynamicMaterials();
    void SetupEquipmentComponents();
    void ApplyArchetypeDefaults(EChar_CharacterArchetype Archetype);
    void UpdatePhysicalTraitsFromArchetype(EChar_CharacterArchetype Archetype);
    void AttachEquipmentToSocket(UStaticMeshComponent* EquipmentComp, const FString& SocketName, const FVector& Offset, const FRotator& Rotation);
    UStaticMeshComponent* CreateEquipmentComponent(const FChar_EquipmentItem& Item);

private:
    // Internal state
    bool bIsInitialized = false;
    bool bNeedsRefresh = false;
};