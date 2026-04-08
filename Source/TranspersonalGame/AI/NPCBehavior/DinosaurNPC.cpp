#include "DinosaurNPC.h"
#include "Components/DinosaurPersonalityComponent.h"
#include "Components/NPCMemorySystem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "AI/NPCBehaviorSystem.h"

// Inicialização do contador estático
int32 ADinosaurNPC::NextDinosaurID = 1;

ADinosaurNPC::ADinosaurNPC()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Gerar ID único
    DinosaurID = NextDinosaurID++;
    
    // Criar componentes especializados
    PersonalityComponent = CreateDefaultSubobject<UDinosaurPersonalityComponent>(TEXT("PersonalityComponent"));
    MemoryComponent = CreateDefaultSubobject<UNPCMemorySystem>(TEXT("MemoryComponent"));
    
    // Configurações padrão
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    SizeVariation = FMath::RandRange(0.8f, 1.2f);
    TrustLevel = 0.0f;
    bIsDomesticated = false;
    
    // Gerar cores aleatórias
    PrimaryColor = FLinearColor::MakeRandomColor();
    SecondaryColor = FLinearColor::MakeRandomColor();
}

void ADinosaurNPC::BeginPlay()
{
    Super::BeginPlay();
    
    // Determinar se pode ser domesticado
    DetermineDomesticationPotential();
    
    // Gerar variações únicas
    GenerateUniqueVariations();
    
    // Gerar nome único
    GenerateUniqueName();
    
    // Registar no sistema de comportamento
    if (UNPCBehaviorSystem* BehaviorSystem = GetWorld()->GetSubsystem<UNPCBehaviorSystem>())
    {
        BehaviorSystem->RegisterDinosaur(this);
    }
    
    // Iniciar timer para comportamentos autónomos
    GetWorld()->GetTimerManager().SetTimer(
        AutonomousBehaviorTimer,
        this,
        &ADinosaurNPC::ExecuteAutonomousBehavior,
        FMath::RandRange(5.0f, 15.0f),
        true
    );
}

void ADinosaurNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Actualizar componentes se existirem
    if (PersonalityComponent)
    {
        PersonalityComponent->UpdatePersonality(DeltaTime);
    }
    
    if (MemoryComponent)
    {
        MemoryComponent->UpdateMemories(DeltaTime);
    }
}

void ADinosaurNPC::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        EDinosaurBehaviorState OldState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;
        
        // Notificar mudança de estado
        OnBehaviorStateChanged(OldState, NewState);
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s (%d) changed state from %d to %d"), 
               *DinosaurName, DinosaurID, (int32)OldState, (int32)NewState);
    }
}

void ADinosaurNPC::IncreaseTrust(float Amount)
{
    if (!bCanBeDomesticated) return;
    
    float OldTrust = TrustLevel;
    TrustLevel = FMath::Clamp(TrustLevel + Amount, 0.0f, 100.0f);
    
    // Verificar se atingiu domesticação
    if (!bIsDomesticated && TrustLevel >= 75.0f)
    {
        bIsDomesticated = true;
        SetBehaviorState(EDinosaurBehaviorState::Domesticated);
        
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s is now domesticated!"), *DinosaurName);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s trust increased from %.1f to %.1f"), 
           *DinosaurName, OldTrust, TrustLevel);
}

void ADinosaurNPC::DecreaseTrust(float Amount)
{
    float OldTrust = TrustLevel;
    TrustLevel = FMath::Clamp(TrustLevel - Amount, 0.0f, 100.0f);
    
    // Perder domesticação se confiança baixar muito
    if (bIsDomesticated && TrustLevel < 50.0f)
    {
        bIsDomesticated = false;
        SetBehaviorState(EDinosaurBehaviorState::Idle);
        
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s is no longer domesticated!"), *DinosaurName);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s trust decreased from %.1f to %.1f"), 
           *DinosaurName, OldTrust, TrustLevel);
}

void ADinosaurNPC::GenerateUniqueVariations()
{
    UniqueFeatures.Empty();
    
    // Gerar características únicas baseadas no tipo
    switch (DinosaurType)
    {
        case EDinosaurType::TyrannosaurusRex:
            if (FMath::RandBool()) UniqueFeatures.Add("Cicatriz profunda no focinho");
            if (FMath::RandBool()) UniqueFeatures.Add("Dente partido visível");
            if (FMath::RandBool()) UniqueFeatures.Add("Postura ligeiramente curvada");
            break;
            
        case EDinosaurType::Triceratops:
            if (FMath::RandBool()) UniqueFeatures.Add("Corno central mais longo");
            if (FMath::RandBool()) UniqueFeatures.Add("Placa craniana com padrão único");
            if (FMath::RandBool()) UniqueFeatures.Add("Cornos laterais assimétricos");
            break;
            
        case EDinosaurType::Compsognathus:
            if (FMath::RandBool()) UniqueFeatures.Add("Cauda com ponta mais escura");
            if (FMath::RandBool()) UniqueFeatures.Add("Movimentos mais ágeis");
            if (FMath::RandBool()) UniqueFeatures.Add("Olhos ligeiramente maiores");
            break;
            
        default:
            if (FMath::RandBool()) UniqueFeatures.Add("Padrão de pele único");
            if (FMath::RandBool()) UniqueFeatures.Add("Tamanho ligeiramente diferente");
            break;
    }
    
    // Aplicar variação de tamanho ao mesh se existir
    if (GetMesh())
    {
        GetMesh()->SetWorldScale3D(FVector(SizeVariation));
    }
}

bool ADinosaurNPC::IsHostileTowards(ADinosaurNPC* OtherDinosaur)
{
    if (!OtherDinosaur || !PersonalityComponent) return false;
    
    // Dinossauros domesticados são menos hostis
    if (bIsDomesticated) return false;
    
    // Verificar se é predador vs presa
    bool bIsPredator = (DinosaurType == EDinosaurType::TyrannosaurusRex ||
                       DinosaurType == EDinosaurType::Allosaurus ||
                       DinosaurType == EDinosaurType::Utahraptor);
                       
    bool bOtherIsPrey = (OtherDinosaur->DinosaurType == EDinosaurType::Compsognathus ||
                        OtherDinosaur->DinosaurType == EDinosaurType::Gallimimus);
    
    if (bIsPredator && bOtherIsPrey)
    {
        return PersonalityComponent->GetAggression() > 0.3f;
    }
    
    // Territorialidade
    float Distance = FVector::Dist(GetActorLocation(), OtherDinosaur->GetActorLocation());
    if (Distance < 500.0f && PersonalityComponent->GetTerritoriality() > 0.7f)
    {
        return true;
    }
    
    return false;
}

bool ADinosaurNPC::ShouldFleeFrom(ADinosaurNPC* OtherDinosaur)
{
    if (!OtherDinosaur || !PersonalityComponent) return false;
    
    // Verificar se o outro é predador
    bool bOtherIsPredator = (OtherDinosaur->DinosaurType == EDinosaurType::TyrannosaurusRex ||
                            OtherDinosaur->DinosaurType == EDinosaurType::Allosaurus ||
                            OtherDinosaur->DinosaurType == EDinosaurType::Spinosaurus);
    
    if (bOtherIsPredator)
    {
        // Herbívoros pequenos sempre fogem de predadores grandes
        bool bIsSmallHerbivore = (DinosaurType == EDinosaurType::Compsognathus ||
                                 DinosaurType == EDinosaurType::Gallimimus);
        
        if (bIsSmallHerbivore)
        {
            return true;
        }
        
        // Outros baseiam-se no medo
        return PersonalityComponent->GetFearfulness() > 0.4f;
    }
    
    return false;
}

void ADinosaurNPC::OnPlayerApproach(float Distance)
{
    if (!PersonalityComponent) return;
    
    if (Distance < 200.0f) // Muito próximo
    {
        if (bIsDomesticated)
        {
            SetBehaviorState(EDinosaurBehaviorState::Socializing);
        }
        else if (PersonalityComponent->GetFearfulness() > 0.6f)
        {
            SetBehaviorState(EDinosaurBehaviorState::Fleeing);
            DecreaseTrust(5.0f);
        }
        else if (PersonalityComponent->GetCuriosity() > 0.7f)
        {
            SetBehaviorState(EDinosaurBehaviorState::Investigating);
        }
    }
    else if (Distance < 500.0f) // Proximidade moderada
    {
        if (PersonalityComponent->GetCuriosity() > 0.5f)
        {
            SetBehaviorState(EDinosaurBehaviorState::Investigating);
        }
    }
}

void ADinosaurNPC::OnPlayerFeedAttempt()
{
    if (!bCanBeDomesticated) return;
    
    // Herbívoros são mais receptivos a alimentação
    bool bIsHerbivore = (DinosaurType == EDinosaurType::Compsognathus ||
                        DinosaurType == EDinosaurType::Triceratops ||
                        DinosaurType == EDinosaurType::Brachiosaurus);
    
    if (bIsHerbivore)
    {
        IncreaseTrust(10.0f);
        SetBehaviorState(EDinosaurBehaviorState::Socializing);
        
        UE_LOG(LogTemp, Log, TEXT("Player fed %s - trust increased"), *DinosaurName);
    }
    else
    {
        // Carnívoros podem ser suspeitos
        if (PersonalityComponent && PersonalityComponent->GetFearfulness() > 0.5f)
        {
            DecreaseTrust(2.0f);
            SetBehaviorState(EDinosaurBehaviorState::Fleeing);
        }
    }
}

void ADinosaurNPC::OnPlayerThreatening()
{
    DecreaseTrust(15.0f);
    
    if (PersonalityComponent)
    {
        if (PersonalityComponent->GetAggression() > 0.7f)
        {
            SetBehaviorState(EDinosaurBehaviorState::Territorial);
        }
        else
        {
            SetBehaviorState(EDinosaurBehaviorState::Fleeing);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Player threatened %s - trust decreased significantly"), *DinosaurName);
}

void ADinosaurNPC::DetermineDomesticationPotential()
{
    // Apenas herbívoros pequenos e médios podem ser domesticados
    switch (DinosaurType)
    {
        case EDinosaurType::Compsognathus:
        case EDinosaurType::Gallimimus:
        case EDinosaurType::Parasaurolophus:
            bCanBeDomesticated = true;
            break;
            
        case EDinosaurType::Triceratops:
        case EDinosaurType::Stegosaurus:
            // Herbívoros médios são mais difíceis
            bCanBeDomesticated = FMath::RandBool(); // 50% chance
            break;
            
        default:
            bCanBeDomesticated = false;
            break;
    }
}

void ADinosaurNPC::GenerateUniqueName()
{
    TArray<FString> NamePrefixes = {
        "Alpha", "Beta", "Gamma", "Delta", "Echo", "Zulu", "Nova", "Orion",
        "Storm", "Thunder", "Lightning", "Shadow", "Flame", "Frost", "Stone", "River"
    };
    
    TArray<FString> NameSuffixes = {
        "claw", "tooth", "horn", "scale", "tail", "eye", "roar", "stride",
        "wing", "spine", "crest", "fang", "hide", "bone", "scar", "mark"
    };
    
    FString Prefix = NamePrefixes[FMath::RandRange(0, NamePrefixes.Num() - 1)];
    FString Suffix = NameSuffixes[FMath::RandRange(0, NameSuffixes.Num() - 1)];
    
    DinosaurName = FString::Printf(TEXT("%s%s-%d"), *Prefix, *Suffix, DinosaurID);
}

void ADinosaurNPC::ExecuteAutonomousBehavior()
{
    if (bIsDomesticated) return; // Dinossauros domesticados não executam comportamentos autónomos
    
    // Escolher comportamento baseado na personalidade
    if (PersonalityComponent)
    {
        TArray<EDinosaurBehaviorState> PossibleStates;
        
        // Estados sempre possíveis
        PossibleStates.Add(EDinosaurBehaviorState::Idle);
        PossibleStates.Add(EDinosaurBehaviorState::Patrolling);
        
        // Estados baseados em personalidade
        if (PersonalityComponent->GetCuriosity() > 0.5f)
        {
            PossibleStates.Add(EDinosaurBehaviorState::Investigating);
        }
        
        if (PersonalityComponent->GetSociability() > 0.6f)
        {
            PossibleStates.Add(EDinosaurBehaviorState::Socializing);
        }
        
        // Estados baseados no tipo
        bool bIsHerbivore = (DinosaurType == EDinosaurType::Compsognathus ||
                            DinosaurType == EDinosaurType::Triceratops ||
                            DinosaurType == EDinosaurType::Brachiosaurus);
        
        if (bIsHerbivore)
        {
            PossibleStates.Add(EDinosaurBehaviorState::Foraging);
        }
        else
        {
            PossibleStates.Add(EDinosaurBehaviorState::Hunting);
        }
        
        // Escolher estado aleatório
        EDinosaurBehaviorState NewState = PossibleStates[FMath::RandRange(0, PossibleStates.Num() - 1)];
        SetBehaviorState(NewState);
    }
    
    // Reagendar próximo comportamento autónomo
    GetWorld()->GetTimerManager().SetTimer(
        AutonomousBehaviorTimer,
        this,
        &ADinosaurNPC::ExecuteAutonomousBehavior,
        FMath::RandRange(10.0f, 30.0f),
        false
    );
}

void ADinosaurNPC::OnBehaviorStateChanged(EDinosaurBehaviorState OldState, EDinosaurBehaviorState NewState)
{
    // Implementação base - pode ser sobrescrita por subclasses
    
    // Registar mudança na memória se o componente existir
    if (MemoryComponent)
    {
        FString EventDescription = FString::Printf(TEXT("Changed behavior from %d to %d"), 
                                                  (int32)OldState, (int32)NewState);
        MemoryComponent->RecordEvent(GetActorLocation(), EventDescription, 0.3f);
    }
}