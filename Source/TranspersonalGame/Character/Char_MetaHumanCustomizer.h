#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_TribalWarriorAsset.h"
#include "Char_MetaHumanCustomizer.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;

/**
 * Component that applies tribal warrior customization to MetaHuman characters
 * Handles mesh swapping, material application, and equipment attachment
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_MetaHumanCustomizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_MetaHumanCustomizer();

protected:
    virtual void BeginPlay() override;

public:
    // Character asset configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Setup")
    UChar_TribalWarriorAsset* CharacterAsset;

    // Target mesh components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Components")
    USkeletalMeshComponent* BodyMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Components")
    UStaticMeshComponent* SpearComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Components")
    UStaticMeshComponent* KnifeComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Components")
    UStaticMeshComponent* NecklaceComponent;

    // Customization methods
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyCharacterAsset();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyBodyCustomization();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyEquipment();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyMaterials();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetHeightScale(float NewScale);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(FLinearColor NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetHairColor(FLinearColor NewHairColor);

    // Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateSetup() const;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FString> GetSetupErrors() const;

private:
    // Internal helper methods
    void AttachEquipmentToSocket(UStaticMeshComponent* Equipment, const FName& SocketName);
    void CreateDynamicMaterialInstance(UMaterialInterface* BaseMaterial, int32 MaterialIndex);
    void ApplyColorParameter(const FName& ParameterName, const FLinearColor& Color, int32 MaterialIndex);

    // Dynamic material instances for runtime customization
    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> DynamicMaterials;
};